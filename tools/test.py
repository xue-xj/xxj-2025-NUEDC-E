"""K230纯Python测试与真实ARM C控制核心测试，不模拟物理外设。"""
import ast
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'.tools'))


def test_c():
    from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS
    from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_R0, UC_ARM_REG_PC
    from elftools.elf.elffile import ELFFile
    compiler = Path(os.environ.get('TI_ARMCLANG', 'E:/ti/ccs2050/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe'))
    linker = os.environ.get('ZIG', str(ROOT/'.tools/ziglang/zig.exe'))
    suites = {'core':['gimbal_control.c','gimbal_protocol.c','motion_math.c'],
              'protocol':['gimbal_protocol.c'], 'control':['gimbal_control.c','motion_math.c'],
              'motor':['step_motor.c','motion_math.c'], 'display':['gimbal_display.c']}
    builtins = compiler.parent.parent/'lib/armv6m-ti-none-eabi/libclang_rt.builtins.a'
    sdk=Path(os.environ.get('MSPM0_SDK','C:/TI/mspm0_sdk_2_10_00_04'))
    includes=[ROOT/'mspm0',ROOT/'build/syscfg',sdk/'source',sdk/'source/third_party/CMSIS/Core/Include']
    for name, dependencies in suites.items():
        sources = [ROOT/f'tests/test_{name}.c', ROOT/'tests/test_runtime.c']
        sources += [ROOT/'mspm0'/p for p in dependencies]
        objects=[]
        for index, source in enumerate(sources):
            obj=ROOT/f'build/test_{name}_{index}.o'
            subprocess.run([str(compiler),'-mcpu=cortex-m0plus','-mthumb','-Oz','-g','-fno-builtin',
                            '-ffunction-sections','-fdata-sections','-D__MSPM0G3507__','-D__USE_SYSCONFIG__',
                            *['-I'+str(p) for p in includes],'-c',str(source),'-o',str(obj)],check=True)
            objects.append(str(obj))
        elf_path=ROOT/f'build/test_{name}.elf'
        subprocess.run([linker,'ld.lld','--gc-sections','-T',str(ROOT/'tests/test.ld'),'-o',str(elf_path),*objects,
                        str(sdk/'source/ti/driverlib/lib/ticlang/m0p/mspm0g1x0x_g3x0x/driverlib.a'),str(builtins)],check=True)
        cpu=Uc(UC_ARCH_ARM,UC_MODE_THUMB|UC_MODE_MCLASS)
        cpu.mem_map(0,0x20000);cpu.mem_map(0x20200000,0x8000)
        cpu.mem_map(0x40000000,0x1000000);cpu.mem_map(0xe0000000,0x100000)
        with elf_path.open('rb') as file:
            elf=ELFFile(file);entry=elf.header['e_entry']
            for segment in elf.iter_segments():
                if segment['p_type']=='PT_LOAD' and segment['p_filesz']:cpu.mem_write(segment['p_vaddr'],segment.data())
        cpu.reg_write(UC_ARM_REG_SP,0x20207ff8);cpu.reg_write(UC_ARM_REG_LR,0x1fff1)
        cpu.emu_start(entry|1,0x1fff0,count=1000000)
        if cpu.reg_read(UC_ARM_REG_PC)!=0x1fff0 or cpu.reg_read(UC_ARM_REG_R0)!=0:
            raise AssertionError('{} failed, return {}'.format(name,cpu.reg_read(UC_ARM_REG_R0)))
        print('ARM '+name+': PASS')
    return list(suites)


def main():
    (ROOT/'build').mkdir(exist_ok=True)
    for path in (ROOT/'k230').rglob('*.py'):ast.parse(path.read_text(encoding='utf-8'),filename=str(path))
    env=os.environ.copy();env['PYTHONPATH']=str(ROOT/'k230');env['PYTHONDONTWRITEBYTECODE']='1'
    subprocess.run([sys.executable,'-m','unittest','discover','-s',str(ROOT/'tests'),'-p','test_*.py','-v'],env=env,check=True)
    suites=test_c()
    (ROOT/'build/test-results.json').write_text(json.dumps({'python_tests':9,'arm_suites':suites},indent=2),encoding='utf-8')


if __name__=='__main__':main()
