<?php
function real_post() {
	static $post;
	if (!isset($post)) {
	  $pairs = explode("&", file_get_contents("php://input"));
	  $post = array();
	  foreach ($pairs as $pair) {
		$x = explode("=", $pair);
		$post[rawurldecode($x[0])] = rawurldecode($x[1]);
	  }
	}
	return $post;
  }
$_POST = real_post();
var_dump($_POST);
$name = $_POST['fname'];
echo $name;
?>