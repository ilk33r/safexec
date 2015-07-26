--TEST--
Check safeexec command execution
--INI--
safeexec.dissallow_sudo_command=1
safeexec.dissallow_all_expect_php=0
--SKIPIF--
<?php if (!extension_loaded("safeexec")) print "skip"; ?>
--FILE--
<?php 
echo system('sudo cat /etc/passwd');

echo system('echo Hello');
?>
--EXPECTF--
Warning: system(): Cannot execute %s command. Sudo command is not allowed with safeexec extension in %s003.php on line %d
Hello
%s