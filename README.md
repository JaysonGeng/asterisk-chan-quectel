Channel driver for Quectel and Simcom modules 
=================================================
forked from IchthysMaranatha/asterisk-chan-quectel


基于原版当使用中国电信sim卡时，由于电信无法注册GSM，原本会一直卡在不断重试注册GSM，导致asterisk无法正确识别sim卡状态，
本项目修改添加通过cereg判断是否注册上4g VoLTE通话更新设备状态 兼容GSM设备
在SIM7600 系列 asterisk v18.x 测试可用 

Building:
----------

    $ ./bootstrap
    $ ./configure --with-astversion=16.20
    $ make
    $ make install
    copy quectel.conf to /etc/asterisk Change context and audio serial port as required

If you run a different version of Asterisk, you'll need to update the
`16.20` as appropriate, obviously.

If you did not `make install` Asterisk in the usual location and configure
cannot find the asterisk header files in `/usr/include/asterisk`, you may
optionally pass `--with-asterisk=PATH/TO/INCLUDE`.

Here is an example for the dialplan:
------------------------------------

**WARNING**: *This example uses the raw SMS message passed to System() directly.
No sane person would do that with untrusted data without escaping/removing the
single quotes.*

    [quectel-incoming]
    exten => sms,1,Verbose(Incoming SMS from ${CALLERID(num)} ${BASE64_DECODE(${SMS_BASE64})})
    exten => sms,n,System(echo '${STRFTIME(${EPOCH},,%Y-%m-%d %H:%M:%S)} - ${QUECTELNAME} - ${CALLERID(num)}: ${BASE64_DECODE(${SMS_BASE64})}' >> /var/log/asterisk/sms.txt)
    exten => sms,n,Hangup()

    exten => ussd,1,Verbose(Incoming USSD: ${BASE64_DECODE(${USSD_BASE64})})
    exten => ussd,n,System(echo '${STRFTIME(${EPOCH},,%Y-%m-%d %H:%M:%S)} - ${QUECTELNAME}: ${BASE64_DECODE(${USSD_BASE64})}' >> /var/log/asterisk/ussd.txt)
    exten => ussd,n,Hangup()

    exten => s,1,Dial(SIP/2001@othersipserver)
    exten => s,n,Hangup()

    [othersipserver-incoming]

    exten => _X.,1,Dial(Quectel/r1/${EXTEN})
    exten => _X.,n,Hangup

    exten => *#123#,1,QuectelSendUSSD(quectel0,${EXTEN})
    exten => *#123#,n,Answer()
    exten => *#123#,n,Wait(2)
    exten => *#123#,n,Playback(vm-goodbye)
    exten => *#123#,n,Hangup()

    exten => _#X.,1,QuectelSendSMS(quectel0,${EXTEN:1},"Please call me",1440,yes,"magicID")
    exten => _#X.,n,Answer()
    exten => _#X.,n,Wait(2)
    exten => _#X.,n,Playback(vm-goodbye)
    exten => _#X.,n,Hangup()

You can also use this:
----------------------

Call using a specific group:

    exten => _X.,1,Dial(Quectel/g1/${EXTEN})

Call using a specific group in round robin:

    exten => _X.,1,Dial(Quectel/r1/${EXTEN})

Call using a specific quectel:

    exten => _X.,1,Dial(Quectel/quectel0/${EXTEN})

Call using a specific provider name:

    exten => _X.,1,Dial(Quectel/p:PROVIDER NAME/${EXTEN})

Call using a specific IMEI:

    exten => _X.,1,Dial(Quectel/i:123456789012345/${EXTEN})

Call using a specific IMSI prefix:

    exten => _X.,1,Dial(Quectel/s:25099203948/${EXTEN})

How to store your own number:

    quectel cmd quectel0 AT+CPBS=\"ON\"
    quectel cmd quectel0 AT+CPBW=1,\"+123456789\",145


Other CLI commands:
-------------------

    quectel reset <device>
    quectel restart gracefully <device>
    quectel restart now <device>
    quectel restart when convenient <device>
    quectel show device <device>
    quectel show devices
    quectel show version
    quectel sms <device> number message
    quectel ussd <device> ussd
    quectel stop gracefully <device>
    quectel stop now <device>
    quectel stop when convenient <device>
    quectel start <device>
    quectel restart gracefully <device>
    quectel restart now <device>
    quectel restart when convenient <device>
    quectel remove gracefully <device>
    quectel remove now <device>
    quectel remove when convenient <device>
    quectel reload gracefully
    quectel reload now
    quectel reload when convenient


