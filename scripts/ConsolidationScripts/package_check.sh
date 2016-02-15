#!/bin/sh

if [ `which sshpass` ];then

	echo "SSH PASS WAS FOUND"
	exit;

else

	echo "I guess I need to install"

fi

cp /opt/openvr/docs/packages/sshpass.deb /tmp

echo "password" | sudo -S dpkg -i /tmp/sshpass.deb

