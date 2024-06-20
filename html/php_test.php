
<html>
  <head><title>Hello World</title></head>
  <body>
<p>ADD: ?hoi=var1&doei=var2 (to test variable passing.)</p>
<?php
echo '<b>Hello</b>';
echo '<b> hoi=';
echo $_GET['hoi'];
echo ' doei= ';
echo $_GET['doei'];
echo '</b>'; #. htmlspecialchars($_SERVER["name"]) . '!';
echo '<p>HOST:';
echo $_SERVER['HOST'];
echo '</p>';
?>
  </body>
</html>
