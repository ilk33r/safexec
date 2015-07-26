# safexec
PHP extension used to prevent sudo command in exec(), system(), passthru(), shell_exec(), proc_open(), popen() commands.

## Requirement
- PHP-5.2 +

### Compile safeexec in Linux
````
$/path/to/phpize
$./configure --enable-safeexec
$make && make install
````

### php.ini directives
* ; Disallow sudo commands
* safeexec.dissallow_sudo_command=1
* ; Disallow all commands except php.
* safeexec.dissallow_all_expect_php=0

### Usage
````
<?php

// This command will cause error.
echo exec('sudo cat /etc/passwd');

// This command will execute normally.
echo exec('whoami');


// if safeexec.dissallow_all_expect_php is on
// This command will cause error.
echo exec('whoami');

// This command will execute normally.
echo exec('php -r "echo \'Hello\';"');
````
