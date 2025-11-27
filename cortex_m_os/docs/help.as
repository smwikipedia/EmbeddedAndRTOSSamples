Usage: /home/ming/dev/toolchains/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-as [option...] [asmfile...]
Options:
  -a[sub-option...]	  turn on listings
                      	  Sub-options [default hls]:
                      	  c      omit false conditionals
                      	  d      omit debugging directives
                      	  g      include general info
                      	  h      include high-level source
                      	  i      include ginsn and synthesized CFI info
                      	  l      include assembly
                      	  m      include macro expansions
                      	  n      omit forms processing
                      	  s      include symbols
                      	  =FILE  list to FILE (must be last sub-option)
  --alternate             initially turn on alternate macro syntax
  --compress-debug-sections[={none|zlib|zlib-gnu|zlib-gabi|zstd}]
                          compress DWARF debug sections
		            Default: none
  --nocompress-debug-sections
                          don't compress DWARF debug sections
  -D                      produce assembler debugging messages
  --dump-config           display how the assembler is configured and then exit
  --debug-prefix-map OLD=NEW
                          map OLD to NEW in debug information
  --defsym SYM=VAL        define symbol SYM to given value
  --execstack             require executable stack for this object
  --noexecstack           don't require executable stack for this object
  --size-check=[error|warning]
			  ELF .size directive check (default --size-check=error)
  --elf-stt-common=[no|yes] (default: no)
                          generate ELF common symbols with STT_COMMON type
  --sectname-subst        enable section name substitution sequences
  --generate-missing-build-notes=[no|yes] (default: no)
                          generate GNU Build notes if none are present in the input
  --gsframe               generate SFrame stack trace information
  -f                      skip whitespace and comment preprocessing
  -g, --gen-debug         generate debugging information
  --gstabs                generate STABS debugging information
  --gstabs+               generate STABS debug info with GNU extensions
  --gdwarf-<N>            generate DWARF<N> debugging information. 2 <= <N> <= 5
  --gdwarf-cie-version=<N> generate version 1, 3 or 4 DWARF CIEs
  --gdwarf-sections       generate per-function section names for DWARF line information
  --hash-size=<N>         ignored
  --help                  show all assembler options
  --target-help           show target specific options
  -I DIR                  add DIR to search list for .include directives
  -J                      don't warn about signed overflow
  -K                      warn when differences altered for long displacements
  -L, --keep-locals       keep local symbols (e.g. starting with `L')
  -M, --mri               assemble in MRI compatibility mode
  --MD FILE               write dependency information in FILE (default none)
  --multibyte-handling=<method>
                          what to do with multibyte characters encountered in the input
  -nocpp                  ignored
  -no-pad-sections        do not pad the end of sections to alignment boundaries
  -o OBJFILE              name the object-file output OBJFILE (default a.out)
  -R                      fold data section into text section
  --reduce-memory-overheads ignored
  --statistics            print various measured statistics from execution
  --strip-local-absolute  strip local absolute symbols
  --traditional-format    Use same format as native assembler when possible
  --version               print assembler version number and exit
  -W, --no-warn           suppress warnings
  --warn                  don't suppress warnings
  --fatal-warnings        treat warnings as errors
  --no-info               suppress information messages
  --info                  don't suppress information messages
  -w                      ignored
  -X                      ignored
  -Z                      generate object file even after errors
  --listing-lhs-width     set the width in words of the output data column of
                          the listing
  --listing-lhs-width2    set the width in words of the continuation lines
                          of the output data column; ignored if smaller than
                          the width of the first line
  --listing-rhs-width     set the max width in characters of the lines from
                          the source file
  --listing-cont-lines    set the maximum number of continuation lines used
                          for the output data column of the listing
  @FILE                   read options from FILE
 ARM-specific assembler options:
  -k                      generate PIC code
  -mthumb                 assemble Thumb code
  -mthumb-interwork       support ARM/Thumb interworking
  -mapcs-32               code uses 32-bit program counter
  -mapcs-26               code uses 26-bit program counter
  -mapcs-float            floating point args are in fp regs
  -mapcs-reentrant        re-entrant code
  -matpcs                 code is ATPCS conformant
  -mbig-endian            assemble for big-endian
  -mlittle-endian         assemble for little-endian
  -mapcs-frame            use frame pointer
  -mapcs-stack-check      use stack size checking
  -mno-warn-deprecated    do not warn on use of deprecated feature
  -mwarn-restrict-it      warn about performance deprecated IT instructions in ARMv8-A and ARMv8-R
  -mwarn-syms             warn about symbols that match instruction names [default]
  -mno-warn-syms          disable warnings about symobls that match instructions
  -mcpu=<cpu name>	  assemble for CPU <cpu name>
  -march=<arch name>	  assemble for architecture <arch name>
  -mfpu=<fpu name>	  assemble for FPU architecture <fpu name>
  -mfloat-abi=<abi>	  assemble for floating point ABI <abi>
  -meabi=<ver>		  assemble for eabi version <ver>
  -mimplicit-it=<mode>	  controls implicit insertion of IT instructions
  -mccs			  TI CodeComposer Studio syntax compatibility mode
  -mfp16-format=[ieee|alternative]
                          set the encoding for half precision floating point numbers to IEEE
                          or Arm alternative format.
  -EB                     assemble code for a big-endian cpu
  -EL                     assemble code for a little-endian cpu
  --fix-v4bx              Allow BX in ARMv4 code
  --fdpic                 generate an FDPIC object file

Report bugs to <https://bugs.linaro.org/>
