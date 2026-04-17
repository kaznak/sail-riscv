import Lake
open Lake DSL System

package «lean-riscv» where
  version := v!"0.1.0"

@[default_target]
lean_lib LeanRiscv

lean_exe lean_riscv_emulator where
  root := `Main

require ELFSage from git
  "https://github.com/GaloisInc/ELFSage.git" @ "7e68e68e836025df741de04a5bb83439fe88d956"

require Lean_RV64D_executable from "../build/model/Lean_RV64D_executable/"
