cmake_minimum_required(VERSION 3.20)
project(ClassicPanel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
# CRT estático: a DLL roda dentro do explorer.exe, sem depender de redistribuível
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG        v1.3.4
)
FetchContent_MakeAvailable(minhook)

# --- DLL
add_library(classic_panel SHARED classic_panel.cpp)
target_include_directories(classic_panel PRIVATE ${minhook_SOURCE_DIR}/include)
target_link_libraries(classic_panel PRIVATE minhook comctl32 psapi dbghelp shell32 delayimp)
# dbghelp é carregado manualmente (cópia local com symsrv.dll) antes do primeiro uso
target_link_options(classic_panel PRIVATE /DELAYLOAD:dbghelp.dll)
set_target_properties(classic_panel PROPERTIES PREFIX "")

# --- Injetor (sem janela de console)
add_executable(ClassicPanelInjector WIN32 injector.cpp)
target_compile_definitions(ClassicPanelInjector PRIVATE UNICODE _UNICODE)
target_link_options(ClassicPanelInjector PRIVATE /ENTRY:wWinMainCRTStartup)
