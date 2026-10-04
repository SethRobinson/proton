# Offline command-line tasks in Windows apps

A Windows application may define `RT_COMMAND_LINE_TASKS` in its project and
override `BaseApp::RunCommandLineTask(const vector<string>& args, int& exitCode)`.
The WinMain entry point calls it with CRT-parsed arguments (excluding the exe
name), preserving quoted paths.

Return `true` when the command is handled: WinMain immediately returns the
specified process exit code. Return `false` to continue normal startup. Apps that
do not define the macro keep their existing startup order.

The callback runs before window creation, input/audio/socket initialization,
configuration loading, or changing the working directory. The App constructor
and its members have run; they must not rely on those subsystems. Do not call GL,
capture, GUI or network functions here. Use ordinary file IO for inputs/results.
Normal `Init`/`Kill` are not called for a handled task, so the callback must release
its own resources. Keep constructor behavior compatible with this early call.

Define an app-specific command prefix and handle invalid commands in that
namespace by returning an error, rather than falling through into the GUI.
Windows GUI-subsystem binaries have no reliable console output; use an output
file plus the process exit status. A calling script should wait for completion
(`subprocess.run`, or PowerShell `Start-Process -Wait -PassThru -WindowStyle Hidden`).

RTMahjong exercises this with saved vision JSON, policy-reply validation and an
AutoManager self-test. Its opt-in startup and quoted-path checks run in both
Debug and Release configurations without video, capture, plotter or desktop UI
automation. That app lives in a separate checkout and is not an engine regression
suite dependency. The hook does not affect rendering, and no renderer golden
files or existing test harness commands are changed.
