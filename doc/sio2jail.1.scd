sio2jail(1)

# NAME

sio2jail - tool for supervising execution of programs
submitted in algorithmic competitions

# SYNOPSIS

*sio2jail* [_options_] [*--*] _executable-path_ [_args_...]

# DESCRIPTION

sio2jail is a tool designed to isolate 3rd party programs while
limiting and measuring the resources (such as CPU instructions
or memory) in a reproducible way, independent of the exact specs
of the hardware sio2jail runs on.

# OPTIONS

*-h, --help*
	Show help message and exit.

*-v, --version*
	Print version and exit.

*-l* _file_, *--log* _file_
	Log to the specified _file_. Use *-* to log to stderr.

*-f* _fd_, *--resultsfd* _fd_
	Write the execution report to file descriptor _fd_,
	instead of stderr.

*-s, --stderr*
	Pass stderr from the sandboxed program,
	instead of redirecting it to stderr.

*-o* _format_, *--output* _format_
	Use the specified _format_ for outputting the execution report.

* --stimelimit* _limit_[*u*|*ms*|*s*|*m*|*h*|*d*] ++
* --utimelimit* _limit_[*u*|*ms*|*s*|*m*|*h*|*d*] ++
*--ustimelimit* _limit_[*u*|*ms*|*s*|*m*|*h*|*d*] ++
* --rtimelimit* _limit_[*u*|*ms*|*s*|*m*|*h*|*d*] ++
	Set system (*stimelimit*), user (*utimelimit*),
	user+system (*ustimelimit*) or real (*rtimelimit*)
	time limit to _limit_.

	Use *u*/*ms*/*s*/*m*/*h*/*d* (case-insensitive) unit suffices
	to specify time in microseconds, miliseconds, seconds, minutes,
	hours and days respectively.
	Defaults to microseconds if unit is not specified.

	Use 0 for no limit (the default).

*--output-limit* _limit_[*b*|*k*|*m*|*g*]
	Set the output file size limit to _limit_.

	Use with *k*/*m*/*g* (case-insensitive) unit suffices
	for 1, 1024, 1024**2, 1024**3 bytes respectively. Default is kibibytes.

	This is currently implemented as an rlimit of maximum created file
	size (See: *RLIMIT\_FSIZE* in *getrlimit*(2)), which means:

	- it doesn't affect stdout unless it's redirected to a file

	- it affects other files written by the sandboxed program
	  not just stdout

	This may change in future versions, so please don't rely on it
	for other purposes than limiting the size of stdout redirected
	to a file.

	Use 0 for no limit (the default).

*--perf* *on*|*off*
	Enable or disable use of perf to measure the number of instructions
	executed by the sandboxed program. Enabled by default.

	See also: *perf\_event\_open*(2)

*--instruction-count-limit* _limit_[*k*|*m*|*g*]
	Set instruction count limit. Requires *--perf*.

	Use with *k*/*m*/*g* sufixes for 10**{3,6,9} respectively.

	Use 0 for no limit (the default).


*--seccomp* *on*|*off*
	Enable or disable use of *seccomp*(2) to block certain syscalls.
	Requires *--ptrace*. Enabled by default.

	Syscall blocking serves several purposes:

	- enforcing contest rules (eg. "programs must not open files")

	- preventing sandboxed programs from evading resource
	  limits/measurement (eg. by creating threads)

	- accurately detecting out-of-memory conditions

	- hardening by disallowing potentially dangerous syscalls
	  which the sandboxed program shouldn't be allowed to use
	  anyway (eg. *mount*(2))

	Currently, there is only one, hardcoded syscall policy.

*--ptrace* *on*|*off*
	Enable or disable use of *ptrace*(2). Enabled by default.

	Ptrace is used for two purposes:

	- restoring normal singal behaviour when pid-namespaces are in use

	- providing seccomp policy more flexibility by using the *TRACE*
	  seccomp action and making the decision whether to allow
	  the syscall in userspace

*-m* _limit_, *--memory-limit* _limit_
	Set memory limit to _limit_. Requires seccomp.

	Use with *k*/*m*/*g* (case-insensitive) unit suffices
	for 1, 1024, 1024**2, 1024**3 bytes respectively. Default is kibibytes.

	Use 0 for no limit.

*--user-namespace* *on*|*off*
	Enable or disable use of *user\_namespaces*(7). Enabled by default.

	This allows sio2jail to use other kinds of namespaces to isolate
	the sandboxed process.

*--capability-drop* *on*|*off*
	Enable or disable dropping capabilities acquired when creating
	a user namespace. Requires *--user-namespace*. Enabled by default.

	This prevents the sandboxed program from manipulating
	the namespaces created by sio2jail, and decreases attack surface
	by dropping unnecessary privileges within the user namespace.

*--mount-namespace* *on*|*off*
	Enable or disable use of *mount\_namespaces*(7).
	Requires *--user-namespace*. Enabled by default.

	This is used to confine the sandboxed process into
	a separate view of the filesystem (kinda like chroot).

	This prevents the sandboxed program from seeing or manipulating
	files which were not explicitely made accessible to it,
	and allows for use of runtime environments different than
	those installed systemwite (eg. different compiler version).

*-b* _path-outside_:_path-inside_[:_flags_]
*--bind* _path-outside_:_path-inside_[:_flags_]
	Bind mount _path-outside_ into the mount namespace
	at mountpoint _path-inside_. Requires *--mount-namespace*.

	This option can be passed multiple times to define multiple
	bind-mounts.

	_path-inside_ must be a valid mountpoint.

	This means that it must be either an empty directory,
	if _path-outside_ is a directory
	or a regular file, if _path-outside_ is a regular file.

	_flags_, if specified, must be of form (*ro*|*rw*)[*,dev*]

	*ro* - mount read-only (the default)

	*rw* - mount read-write

	*dev* - allow the mounted file to behave as a device node

	By default, unless *-B* is specified, the file to be executed
	is mounted read-only at /exe, as if the following was passed:

	\-b _executable_:/exe:ro

*-B, --no-default-binds*
	Don't add default bind-mounts (i.e. binding the executable at /exe).
	Makes sense only if *--mount-namespace* is enabled.

	See *-b*.

*--pid-namespace* *on*|*off*
	Enable or disable use of *pid\_namespaces*(7).
	Requires *--user-namespace*. Enabled by default.

*--procfs* *on*|*off*
	Enables mounting *procfs*(5) at /proc inside the sandbox.
	Requires *--pid-namespace* and *--mount-namespace*.
	Disabled by default.

	In most cases, it's not necessary to make procfs available
	to programs inside the sandbox, and not providing it decreases
	the attack surface.

	However, procfs is sometimes useful when debugging, and some
	programs may require it to function.

	When PID namespaces are in use, the PIDs inside the sandbox
	are different than outside, so new procfs needs to be mounted
	inside the sandbox, as the procfs outside contains inadequate
	information.

	Use of PID namespaces without mount namespaces is not recommended,
	as it makes incorrect procfs always visible to the sandboxed process.

	In a production environment, for better security, this option should
	remain disabled.

*--uts-namespace* *on*|*off*
	Enable or disable use of UTS namespaces to eliminate the impact of
	hostname and other UTS metadata on the sandboxed program.
	Requiers *--user-namespace*. Enabled by default.

	When enabled, this option sets the hostname and domainname
	inside the sandbox to "sio2jail".

*--net-namespace* *on*|*off*
	Enable or disable use of *network\_namespaces*(7).
	Requires *--user-namespace*. Enabled by default.

	This confines the sandboxed program to a view of network
	where there is no network interfaces except for a new
	instance of the loopback interface, separate from
	the hosts's loopback interface.

	In other words, the sandboxed program has its own
	network isolated from anything outside the sandbox.

*--ipc-namespace* *on*|*off*
	Enable or disable the ose of IPC namespaces.
	Requires *--user-namespace*. Enabled by default.

	This confines the sandboxed program to a view of IPC
	objects separate from any IPC objects outside.


# EXIT STATUS

*sio2jail* exits with status 0 if the sandboxed program finished executing
without any error on sio2jail's side.

Exit status 1 indicates that the supplied arguments are wrong.

Exit status 2 indicates a fatal error during startup or runtime.

# SEE ALSO

*namespaces*(7), *seccomp*(2), *ptrace*(2), *mount*(8), *capabilities*(7),
*credentials*(7), *procfs*(5), *getrlimit*(2), *times*(2)

# AUTHORS

Written by Wojciech Dubiel, Tadeusz Dudkiewicz, Przemysław Jakub Kozłowski, Maciej Wachulec.
Maintained by Wojciech Dubiel <wd371280@students.mimuw.edu.pl>.
Up-to-date sources can be found at https://github.com/sio2project/sio2jail/
