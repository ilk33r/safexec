--TEST--
Check safeexec command execution
--INI--
safeexec.dissallow_sudo_command=1
safeexec.dissallow_all_expect_php=0
--SKIPIF--
<?php if (!extension_loaded("safeexec")) print "skip"; ?>
--FILE--
<?php 

echo exec();

?>
--EXPECTF--
Warning: exec() expects at least 1 parameter, 0 given in %s008.php on line %d