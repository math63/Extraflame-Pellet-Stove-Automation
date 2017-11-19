# Extraflame-Pellet-Stove-Automation


https://pcbs.io/share/42JMy

https://www.sculpteo.com/fr/print_old/boite_v2-10/cqvDiYiF?basket=1&noclickredirect=1&uuid=6ML2Aqv1MtEUXSKbfupsUg

https://circuits.io/circuits/5604341-arduinorfm95-v6

Comments :
In mysensors plugin for jeedom, change baudrate from 115200 to 38400 :
Mysensors/Node/mysensors.js


var SerialPort = require('serialport');
	gw = new SerialPort(gwAddress);
        //compatibilité avec la nouvelle verion de serialport
        if ( gw.settings.baudRate ){
                gw.settings.baudRate=38400;
        }else{
                gw.settings.baudrate=38400;
        }
