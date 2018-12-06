# ODin_dataServer
Host side data logging server for ODIn (OD reporting)

#### Setup instructions for Pi
* clone repo into `/var/www/ODIn_dataServer`
* build with `make`
* change the owner to the www service user account `chown -R www-data.www-data /var/www/ODIn_dataServer`
* setup apache server (see /etc/apache2)
   * modify this file /etc/apache2/ports.conf so that the line
```
Listen 80
```
is changed to
```
Listen 1441
```
   * modify this file /etc/apache2/sites-available/000-default.conf to contain
```
<VirtualHost *:1441>
        ServerAdmin webmaster@localhost
        DocumentRoot /var/www/ODIn_dataServer
        ErrorLog ${APACHE_LOG_DIR}/error.log
        CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>
```
* setup startup script, we are going to use the simple approach with `/etc/rc.local`; add this line to that file before the `exit 0`
`sudo -u www-data /var/www/ODIn_dataServer/run&`
