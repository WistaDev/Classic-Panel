# Classic Panel (standalone)

Port do mod Windhawk `cpl-reborn` (aubymori) para **DLL + injetor `.exe`**, sem depender do Windhawk.

## Arquivos
- `classic_panel.cpp` – a DLL (lógica do mod, MinHook, resolução de símbolo via dbghelp)
- `injector.cpp` – `ClassicPanelInjector.exe`, fica em segundo plano e injeta a DLL em
  `explorer.exe`, `control.exe`, `mblctr.exe`, `rundll32.exe`, `DpiScaling.exe`
- `CMakeLists.txt` – build (baixa o MinHook sozinho)

## Compilar (Visual Studio 2022, x64)
Abra o "x64 Native Tools Command Prompt for VS 2022":

```
cmake -S . -B build -A x64
cmake --build build --config Release
```

Saída: `build\Release\classic_panel.dll` e `build\Release\ClassicPanelInjector.exe`.
Precisa de CMake e Git instalados.

## Preparar a pasta de execução
Copie para **a mesma pasta** (ex.: `C:\ClassicPanel\`):

1. `classic_panel.dll`
2. `ClassicPanelInjector.exe`
3. `dbghelp.dll` **e** `symsrv.dll` de
   `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\`
   (Debugging Tools for Windows, vem no Windows SDK). O `dbghelp.dll` do System32
   não baixa símbolos, por isso essas duas cópias são necessárias.

## Usar
1. Execute `ClassicPanelInjector.exe`. Ele não abre janela.
2. Na **primeira** execução a DLL baixa o PDB do `shell32.dll` do Symbol Server da
   Microsoft (internet necessária, pode levar alguns segundos). Fica em cache em
   `%LOCALAPPDATA%\ClassicPanel\symbols`.
3. Abra o Painel de Controle (`control.exe`).
4. Para iniciar com o Windows: coloque um atalho do injetor em `shell:startup`.
5. Para parar: encerre `ClassicPanelInjector.exe` no Gerenciador de Tarefas e reinicie o
   `explorer.exe` (a DLL permanece nos processos já injetados até eles fecharem).

## Depuração
Use o DebugView (Sysinternals) e filtre por `[ClassicPanel]`. Mensagens esperadas:
`Found "..."` para as strings e `Hook ativo em ...`.

## Observações
- Só x64. Não injeta em processos 32 bits.
- Os offsets `+520`, `HDPA` em `+2*ptr` e categoria em `+32*DWORD` são os mesmos do mod
  original, ou seja, dependem do build do `shell32.dll` (o mod é feito para Windows 10).
  Em outra versão do Windows pode não funcionar ou travar o Painel de Controle.
- Antivírus costumam sinalizar injetores de DLL (falso positivo). Pode ser preciso criar exceção.
- Rodar como administrador não é necessário para processos do seu usuário.
- Não compilei nem testei aqui (sem ambiente Windows), então pode haver ajustes pequenos de compilação.