--TEST--
Check safeexec command execution
--INI--
safeexec.dissallow_sudo_command=1
safeexec.dissallow_all_expect_php=0
--SKIPIF--
<?php if (!extension_loaded("safeexec")) print "skip"; ?>
--FILE--
<?php 
echo exec('sudo cat /etc/passwd');

echo exec('echo Hello');
?>
--EXPECTF--
Warning: exec(): Cannot execute %s command. Sudo command is not allowed with safeexec extension in %s002.php on line %d
Hello