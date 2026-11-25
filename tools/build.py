"""TI SysConfig生成、TI ArmClang编译及链接；不自动烧录。"""
import json
import os
from pathlib import Path
import subprocess
import struct

ROOT = Path(__file__).resolve().parents[1]


def export_firmware(path):
    data = path.read_bytes()
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', data)
    chunks = []
    for index in range(header[10]):
        segment = struct.unpack_from('<IIIIIIII', data, header[5] + index * header[9])
        kind, offset, virtual, physical, size, memory, flags, align = segment
        if kind == 1 and size and physical < 0x20000:
            if physical + size > 0x20000:
                raise ValueError('Flash capacity exceeded')
            chunks.append((physical, data[offset:offset+size]))
    image = bytearray(b'\xff' * max(address + len(chunk) for address, chunk in chunks))
    for address, chunk in chunks:
        image[address:address+len(chunk)] = chunk
    path.with_suffix('.bin').write_bytes(image)
    def record(address, kind, payload):
        raw = bytes((len(payload), address >> 8, address & 255, kind)) + payload
        return ':' + (raw + bytes(((-sum(raw)) & 255,))).hex().upper()
    lines, upper = [], None
    for address in range(0, len(image), 16):
        if address >> 16 != upper:
            upper = address >> 16
            lines.append(record(0, 4, upper.to_bytes(2, 'big')))
        lines.append(record(address & 65535, 0, image[address:address+16]))
    lines.append(record(0, 1, b''))
    path.with_suffix('.hex').write_text('\n'.join(lines)+'\n', encoding='ascii')


def build():
    sdk = Path(os.environ.get('MSPM0_SDK', 'C:/TI/mspm0_sdk_2_10_00_04'))
    sysconfig = Path(os.environ.get('SYSCONFIG_ROOT', 'C:/TI/sysconfig_1.26.2'))
    compiler = Path(os.environ.get('TI_ARMCLANG', 'E:/ti/ccs2050/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe'))
    output = ROOT/'build'
    generated = output/'syscfg'
    generated.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(sysconfig/'nodejs/node.exe'), str(sysconfig/'dist/cli.js'),
                    '--product', str(sdk/'.metadata/product.json'), '--compiler', 'ticlang',
                    '--script', str(ROOT/'mspm0/empty.syscfg'), '--output', str(generated)], check=True)
    names = json.loads((ROOT/'mspm0/sources.json').read_text(encoding='utf-8'))
    if len(names)!=len(set(names)) or set(names)!={p.name for p in (ROOT/'mspm0').glob('*.c')}:
        raise ValueError('sources.json must list every application C source exactly once')
    sources = [ROOT/'mspm0'/name for name in names]
    sources += [generated/'ti_msp_dl_config.c', sdk/'source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c']
    includes = [ROOT/'mspm0', generated, sdk/'source', sdk/'source/third_party/CMSIS/Core/Include']
    flags = ['-march=thumbv6m', '-mcpu=cortex-m0plus', '-mfloat-abi=soft', '-mlittle-endian', '-mthumb', '-Oz', '-g',
             '-Wall', '-ffunction-sections', '-fdata-sections', '-D__MSPM0G3507__', '-D__USE_SYSCONFIG__']
    objects, warnings = [], []
    for source in sources:
        obj = output/(source.stem+'.o')
        result = subprocess.run([str(compiler), *flags, *['-I'+str(p) for p in includes], '-c', str(source), '-o', str(obj)],
                                capture_output=True, text=True, encoding='utf-8', errors='replace')
        if result.returncode:
            raise RuntimeError(str(source)+'\n'+result.stdout+result.stderr)
        if result.stderr:
            warnings.append(source.name+'\n'+result.stderr)
        objects.append(str(obj))
    (output/'compile-warnings.txt').write_text('\n'.join(warnings), encoding='utf-8')
    library = compiler.parent.parent/'lib'
    subprocess.run([str(compiler), *flags[:5], *objects, '-Wl,-m'+str(output/'gimbal.map'),
                    '-Wl,-i'+str(sdk/'source'), '-Wl,-i'+str(library),
                    '-Wl,-l'+str(generated/'device_linker.cmd'), '-Wl,-l'+str(generated/'device.cmd.genlibs'),
                    '-Wl,-llibc.a', '-Wl,--rom_model', '-o', str(output/'gimbal.out')], check=True)
    export_firmware(output/'gimbal.out')
    print('TI build passed: {} C files, {} warning files; OUT/BIN/HEX generated'.format(len(sources), len(warnings)))


if __name__ == '__main__':
    build()
