# Import CGI and CGIT module 
import cgi, cgitb 
from os import listdir             

def	add_button(root, title):
	print ("""<button onclick = "window.location.href='%s/%s';">%s</button>""" % (directory, title, title))

  
#Load variables and read all files and dirs from current dir, should use dir which is passed
form = cgi.FieldStorage()
directory = form.getvalue('directory')
dir = listdir("./" + directory);

#SHOW Start:
print ("<!DOCTYPE html>") 
print ("<html>") 
print ("<head>") 
print ("<title>%s</title>" % (directory)) 
print ("</head>") 
print ("<body>") 

#List all files and irectories in directory:
print ("<h1>Directory view of: %s</h1>" % (directory))
print ("<p>Files and directories:</p>")
for title in dir:
	add_button(directory, title)

#Upload files
print ("<p>Upload file.</p>")
#/action_page.php might have to be replaced with another script, to upload files
print ("""<form action="/action_page.php">
  <input type="file" id="myFile" name="filename">
  <input type="submit">
</form>""")
print ("</body>") 
print ("</html>") 
