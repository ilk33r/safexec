--TEST--
Check safeexec command execution
--INI--
safeexec.dissallow_sudo_command=1
safeexec.dissallow_all_expect_php=0
--SKIPIF--
<?php if (!extension_loaded("safeexec")) print "skip"; ?>
--FILE--
<?php 
$descriptorspec = array(
	0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
	1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
	2 => array("file", "/tmp/error-output.txt", "a") // stderr is a file to write to
);

$process = proc_open('sudo su', $descriptorspec, $pipes);

if (is_resource($process)) {
	print stream_get_contents($pipes[0]);
	print stream_get_contents($pipes[1]);
	proc_close($process);
}

$process2 = proc_open('php', $descriptorspec, $pipes);

if (is_resource($process2)) {

	fwrite($pipes[0], '<?php print_r(\'Hello\'); ?>');
	fclose($pipes[0]);

	echo stream_get_contents($pipes[1]);
	fclose($pipes[1]);

	proc_close($process2);
}

?>
--EXPECTF--
Warning: proc_open(): Cannot execute %s command. Sudo command is not allowed with safeexec extension in %s007.php on line %d
Hello