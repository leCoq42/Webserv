# Use the official Apache image from the Docker Hub
FROM httpd:2.4

# Install necessary packages
RUN apt-get update && \
	apt-get install -y g++ && \
	apt-get clean

# Enable CGI module
RUN sed -i 's/#LoadModule cgid_module/LoadModule cgid_module/' /usr/local/apache2/conf/httpd.conf
RUN echo "ScriptAlias /cgi-bin/ /usr/local/apache2/cgi-bin/" >> /usr/local/apache2/conf/httpd.conf
RUN echo "<Directory \"/usr/local/apache2/cgi-bin\">" >> /usr/local/apache2/conf/httpd.conf
RUN echo "    AllowOverride None" >> /usr/local/apache2/conf/httpd.conf
RUN echo "    Options +ExecCGI" >> /usr/local/apache2/conf/httpd.conf
RUN echo "    AddHandler cgi-script .cgi .pl .py .sh .out" >> /usr/local/apache2/conf/httpd.conf
RUN echo "    Require all granted" >> /usr/local/apache2/conf/httpd.conf
RUN echo "</Directory>" >> /usr/local/apache2/conf/httpd.conf

# Copy the C++ CGI script to the cgi-bin directory
COPY time.cgi /usr/local/apache2/cgi-bin/time.cgi

# Make the CGI script executable
RUN chmod +x /usr/local/apache2/cgi-bin/time.cgi

