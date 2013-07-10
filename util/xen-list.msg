Return-path: <xen-tools-discuss-bounces@xen-tools.org>
Envelope-to: steve@steve.org.uk
Delivery-date: Wed, 10 Jul 2013 11:49:41 +0100
Received: from qpsmtpd (helo=ssh.steve.org.uk)
	by ssh.steve.org.uk with local-bsmtp (Exim 4.80)
	(envelope-from <xen-tools-discuss-bounces@xen-tools.org>)
	id 1Uwrxt-0006yW-39
	for steve@steve.org.uk; Wed, 10 Jul 2013 11:49:41 +0100
Received: from sym2.noone.org (HELO sym2.noone.org) (178.63.92.236)
    by mail.steve.org.uk (qpsmtpd/0.84) with ESMTP; Wed, 10 Jul 2013 11:49:41 +0100
Received: from sym2.noone.org (sym [127.0.0.1])
	by sym2.noone.org (Postfix) with ESMTP id C777B135E018;
	Wed, 10 Jul 2013 12:49:39 +0200 (CEST)
X-Original-To: xen-tools-discuss@xen-tools.org
Delivered-To: xen-tools-discuss@sym.noone.org
Received: from mail-la0-f54.google.com (mail-la0-f54.google.com
	[209.85.215.54]) (using TLSv1 with cipher RC4-SHA (128/128 bits))
	(No client certificate requested)
	by sym2.noone.org (Postfix) with ESMTPS id 9673E135E013
	for <xen-tools-discuss@xen-tools.org>;
	Wed, 10 Jul 2013 12:49:37 +0200 (CEST)
Received: by mail-la0-f54.google.com with SMTP id ec20so5708800lab.27
	for <xen-tools-discuss@xen-tools.org>;
	Wed, 10 Jul 2013 03:49:37 -0700 (PDT)
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=gmail.com; s=20120113;
	h=mime-version:in-reply-to:references:date:message-id:subject:from:to
	:content-type; bh=/FdXZf43dd7OW6WaBeCKIFWo58dx7DDNeU2wohDv/H0=;
	b=rXgouDvVgeurfznMMX+qIwMZbBeoznYzFbGavZFWX2FGE+gFEHogW1VIveVHlGyDb6
	JrBjHRM4XT1jM3ao+kkzlwTH8a9TJ85wPBqko4s+dKxBM45jSLjgggY3fihZZP8EiREr
	00lCxjv1ki9Hswh/dndFCs3OkZ0nojnssHxq7FB31amZetyb32Zl3avHARc9MaUU9srt
	MqpXbNVSf5601S+FmFqLpOOo2SVHntoVg9Z+6bWE/qc4Xe+ZfIHqdYuAi/ztSXupLILO
	SBN0UAeAR3jmpIEYI05zeQkythn2nuwbyFCclcM+4JQ5nzQzVs3zMe4XhiRjfRqIUBa4
	+5gA==
MIME-Version: 1.0
X-Received: by 10.152.4.194 with SMTP id m2mr14678980lam.23.1373453376809;
	Wed, 10 Jul 2013 03:49:36 -0700 (PDT)
Received: by 10.112.145.129 with HTTP; Wed, 10 Jul 2013 03:49:36 -0700 (PDT)
In-Reply-To: <20130709142043.GH30957@sym.noone.org>
References: <CAOySv3p1AACbA2Y9cP-UsKGK8h+fxH4pdhTzdnFngjnA_x1gHA@mail.gmail.com>
	<20130709142043.GH30957@sym.noone.org>
Date: Wed, 10 Jul 2013 12:49:36 +0200
Message-ID: <CAOySv3qxDK3VTKQpNBYCSU2uS_c55JPKxEZ_HBhRB_pUz=qd_w@mail.gmail.com> (sfid-20130710_114941_178559_51E8BE78)
From: Kevin FAUCHON <kevin.fauchon@gmail.com>
To: xen-tools-discuss@xen-tools.org, 623443@bugs.debian.org
Subject: Re: [xen-tools-discuss] Bug? use of apt proxy with xen-create-image
X-BeenThere: xen-tools-discuss@xen-tools.org
X-Mailman-Version: 2.1.13
Precedence: list
List-Id: xen-tools discussion mailing list <xen-tools-discuss.xen-tools.org>
List-Unsubscribe: <https://xen-tools.org/mailman/options/xen-tools-discuss>,
	<mailto:xen-tools-discuss-request@xen-tools.org?subject=unsubscribe>
List-Archive: <http://xen-tools.org/pipermail/xen-tools-discuss>
List-Post: <mailto:xen-tools-discuss@xen-tools.org>
List-Help: <mailto:xen-tools-discuss-request@xen-tools.org?subject=help>
List-Subscribe: <https://xen-tools.org/mailman/listinfo/xen-tools-discuss>,
	<mailto:xen-tools-discuss-request@xen-tools.org?subject=subscribe>
Content-Type: multipart/mixed; boundary="===============3782814577050188526=="
Sender: xen-tools-discuss-bounces@xen-tools.org
Errors-To: xen-tools-discuss-bounces@xen-tools.org
X-HELO: sym2.noone.org
X-REMOTE-IP: 178.63.92.236
X-REMOTE-HOST: sym2.noone.org
X-MAIL-FROM: <xen-tools-discuss-bounces@xen-tools.org>
X-RCPT-TO: <steve@steve.org.uk>
X-DOMAIN: steve.org.uk
X-LOCALPART: steve
X-CRM114-Version: 20100106-BlameMichelson ( TRE 0.8.0 (BSD) ) MR-27CA1CFB 
X-CRM114-CacheID: sfid-20130710_114941_178559_51E8BE78 
X-CRM114-Status: GOOD (  29.85  )

--===============3782814577050188526==
Content-Type: multipart/alternative; boundary=089e013d1742a3f02904e12608bf

--089e013d1742a3f02904e12608bf
Content-Type: text/plain; charset=ISO-8859-1

Hello,

===> problem SOLVED <===

>Can you try --apt_proxy=http://88.191.190.70:3142/ instead?
>"=url" implies (admittedly not very clearly) that a protocol needs to
>be included.

Thanks you very much, that's the error.
Nota: tested and OK, with and without the ending slash on proxy url:
http://AA.BB.CC.DD:3142
http://AA.BB.CC.DD:3142/


RESUME: Use of apt-cacher-ng during xen-create-image :

use the option --apt_proxy like this:
  --apt_proxy=http://AA.BB.CC.DD:3142/

Or set it in /etc/xen-tools/xen-tools.conf like that:
  mirror = http://ftp.fr.debian.org/debian/
  apt_proxy = http://AA.BB.CC.DD:3142/


Sorry for not finding this easy solving myself...
I was not sure if it was a bug or not, that's why a bug report was not
created.

Thanks you very much, Kevin.


2013/7/9 Axel Beckert <abe@deuxchevaux.org>

> Control: found 623443 0.9.8.2
> Control: found 623443 0.9.9
>
> Hi Kevin,
>
> On Tue, Jul 09, 2013 at 01:53:31PM +0200, Kevin FAUCHON wrote:
> > The actual manpage of debian wheezy say:
> > "--apt_proxy=url  Specify a proxy to be used by debootstrap, and within
> > the guest."
> >
> > So i launch this command:
> > xen-create-image --apt_proxy=88.191.190.70:3142 --hostname=test2
> > --size=2048Mb \
> > --memory=128Mb --swap=32Mb --ip=192.168.254.98 --role=udev
>
> Can you try --apt_proxy=http://88.191.190.70:3142/ instead?
>
> "=url" implies (admittedly not very clearly) that a protocol needs to
> be included.
>
> > And near the end of the logfile, these errors:
> > W: Failed to fetch
> > http://ftp.fr.debian.org/debian/dists/wheezy/main/binary-amd64/Packages
> > Cannot initiate the connection to 3142:80 (0.0.12.70). - connect (22:
> > Invalid argument)
>
> Fits: It took 88.191.190.70 as protocol (i.e. before the colon) and
> 3142 as hostname (behind the colon). But it's strange that it was able
> to look up 3142 as hostname (or use it aas IP). But then again
> 0.0.12.70 is not a valid IP address anyway and hence it fails.
>
> On a first glance I'd say that this is an APT bug because it doesn't
> parse the setting properly before using it. Digging a little deeper my
> suspicion got confirmed as the issue already has been reported at
> http://bugs.debian.org/623443 (Cc'ed) and I could reproduce the issue
> on Debian Testing with apt 0.9.8.2 as well as on Debian Unstable with
> apt 0.9.9:
>
>   Cannot initiate the connection to 3142:80 (0.0.12.70). - connect (22:
> Invalid argument)
>
> I'll update the xen-tools documentation with regards to that and may
> even add such a check to xen-create-image so that it fails earlier in
> cases like your's. So thanks for the report despite the bug sits
> elsewhere than you suspected.
>
>                 Kind regards, Axel
> --
> /~\  Plain Text Ribbon Campaign                   | Axel Beckert
> \ /  Say No to HTML in E-Mail and News            | abe@deuxchevaux.org (Mail)
>  X   See http://www.asciiribbon.org/              | abe@noone.org(Mail+Jabber)
> / \  I love long mails: http://email.is-not-s.ms/ | http://noone.org/abe/(Web)
> _______________________________________________
> xen-tools-discuss mailing list
> xen-tools-discuss@xen-tools.org
> https://xen-tools.org/mailman/listinfo/xen-tools-discuss
>

--089e013d1742a3f02904e12608bf
Content-Type: text/html; charset=ISO-8859-1
Content-Transfer-Encoding: quoted-printable

Hello,<br><br>=3D=3D=3D&gt; problem SOLVED &lt;=3D=3D=3D<br><br>&gt;Can you=
 try --apt_proxy=3D<a href=3D"http://88.191.190.70:3142/" target=3D"_blank"=
>http://88.191.190.70:3142/</a> instead?<br>
&gt;&quot;=3Durl&quot; implies (admittedly not very clearly) that a protoco=
l needs to<br>
&gt;be included.<br><br>Thanks you very much, that&#39;s the error.<br>Nota=
: tested and OK, with and without the ending slash on proxy url:<br><a href=
=3D"http://AA.BB.CC.DD:3142">http://AA.BB.CC.DD:3142</a><br><a href=3D"http=
://AA.BB.CC.DD:3142/">http://AA.BB.CC.DD:3142/</a><br>
<br><br>RESUME: Use of apt-cacher-ng during xen-create-image :<br><br>use t=
he option --apt_proxy like this:<br>=A0 --apt_proxy=3D<a href=3D"http://AA.=
BB.CC.DD:3142/">http://AA.BB.CC.DD:3142/</a><br><br>Or set it in /etc/xen-t=
ools/xen-tools.conf like that:<br>
=A0 mirror =3D <a href=3D"http://ftp.fr.debian.org/debian/">http://ftp.fr.d=
ebian.org/debian/</a><br>=A0 apt_proxy =3D <a href=3D"http://AA.BB.CC.DD:31=
42/">http://AA.BB.CC.DD:3142/</a><br><br><br>Sorry for not finding this eas=
y solving myself...<br>
I was not sure if it was a bug or not, that&#39;s why a bug report was not =
created.<br><br>Thanks you very much, Kevin.<br><br><br><div class=3D"gmail=
_quote">2013/7/9 Axel Beckert <span dir=3D"ltr">&lt;<a href=3D"mailto:abe@d=
euxchevaux.org" target=3D"_blank">abe@deuxchevaux.org</a>&gt;</span><br>
<blockquote class=3D"gmail_quote" style=3D"margin:0 0 0 .8ex;border-left:1p=
x #ccc solid;padding-left:1ex">Control: found 623443 0.9.8.2<br>
Control: found 623443 0.9.9<br>
<br>
Hi Kevin,<br>
<div class=3D"im"><br>
On Tue, Jul 09, 2013 at 01:53:31PM +0200, Kevin FAUCHON wrote:<br>
&gt; The actual manpage of debian wheezy say:<br>
&gt; &quot;--apt_proxy=3Durl =A0Specify a proxy to be used by debootstrap, =
and within<br>
&gt; the guest.&quot;<br>
&gt;<br>
&gt; So i launch this command:<br>
&gt; xen-create-image --apt_proxy=3D<a href=3D"http://88.191.190.70:3142" t=
arget=3D"_blank">88.191.190.70:3142</a> --hostname=3Dtest2<br>
&gt; --size=3D2048Mb \<br>
&gt; --memory=3D128Mb --swap=3D32Mb --ip=3D192.168.254.98 --role=3Dudev<br>
<br>
</div>Can you try --apt_proxy=3D<a href=3D"http://88.191.190.70:3142/" targ=
et=3D"_blank">http://88.191.190.70:3142/</a> instead?<br>
<br>
&quot;=3Durl&quot; implies (admittedly not very clearly) that a protocol ne=
eds to<br>
be included.<br>
<div class=3D"im"><br>
&gt; And near the end of the logfile, these errors:<br>
&gt; W: Failed to fetch<br>
&gt; <a href=3D"http://ftp.fr.debian.org/debian/dists/wheezy/main/binary-am=
d64/Packages" target=3D"_blank">http://ftp.fr.debian.org/debian/dists/wheez=
y/main/binary-amd64/Packages</a><br>
&gt; Cannot initiate the connection to <a href=3D"tel:3142" value=3D"+33314=
2">3142</a>:80 (0.0.12.70). - connect (22:<br>
&gt; Invalid argument)<br>
<br>
</div>Fits: It took 88.191.190.70 as protocol (i.e. before the colon) and<b=
r>
<a href=3D"tel:3142" value=3D"+333142">3142</a>=A0as hostname (behind the c=
olon). But it&#39;s strange that it was able<br>
to look up <a href=3D"tel:3142" value=3D"+333142">3142</a> as hostname (or =
use it aas IP). But then again<br>
0.0.12.70 is not a valid IP address anyway and hence it fails.<br>
<br>
On a first glance I&#39;d say that this is an APT bug because it doesn&#39;=
t<br>
parse the setting properly before using it. Digging a little deeper my<br>
suspicion got confirmed as the issue already has been reported at<br>
<a href=3D"http://bugs.debian.org/623443" target=3D"_blank">http://bugs.deb=
ian.org/623443</a> (Cc&#39;ed) and I could reproduce the issue<br>
on Debian Testing with apt 0.9.8.2 as well as on Debian Unstable with<br>
apt 0.9.9:<br>
<div class=3D"im"><br>
=A0 Cannot initiate the connection to <a href=3D"tel:3142" value=3D"+333142=
">3142</a>:80 (0.0.12.70). - connect (22: Invalid argument)<br>
<br>
</div>I&#39;ll update the xen-tools documentation with regards to that and =
may<br>
even add such a check to xen-create-image so that it fails earlier in<br>
cases like your&#39;s. So thanks for the report despite the bug sits<br>
elsewhere than you suspected.<br>
<br>
=A0 =A0 =A0 =A0 =A0 =A0 =A0 =A0 Kind regards, Axel<br>
<span class=3D"HOEnZb"><font color=3D"#888888">--<br>
/~\ =A0Plain Text Ribbon Campaign =A0 =A0 =A0 =A0 =A0 =A0 =A0 =A0 =A0 | Axe=
l Beckert<br>
\ / =A0Say No to HTML in E-Mail and News =A0 =A0 =A0 =A0 =A0 =A0| <a href=
=3D"mailto:abe@deuxchevaux.org">abe@deuxchevaux.org</a> =A0(Mail)<br>
=A0X =A0 See <a href=3D"http://www.asciiribbon.org/" target=3D"_blank">http=
://www.asciiribbon.org/</a> =A0 =A0 =A0 =A0 =A0 =A0 =A0| <a href=3D"mailto:=
abe@noone.org">abe@noone.org</a> (Mail+Jabber)<br>
/ \ =A0I love long mails: <a href=3D"http://email.is-not-s.ms/" target=3D"_=
blank">http://email.is-not-s.ms/</a> | <a href=3D"http://noone.org/abe/" ta=
rget=3D"_blank">http://noone.org/abe/</a> (Web)<br>
_______________________________________________<br>
xen-tools-discuss mailing list<br>
<a href=3D"mailto:xen-tools-discuss@xen-tools.org">xen-tools-discuss@xen-to=
ols.org</a><br>
<a href=3D"https://xen-tools.org/mailman/listinfo/xen-tools-discuss" target=
=3D"_blank">https://xen-tools.org/mailman/listinfo/xen-tools-discuss</a><br=
>
</font></span></blockquote></div><br>

--089e013d1742a3f02904e12608bf--

--===============3782814577050188526==
Content-Type: text/plain; charset="us-ascii"
MIME-Version: 1.0
Content-Transfer-Encoding: 7bit
Content-Disposition: inline

_______________________________________________
xen-tools-discuss mailing list
xen-tools-discuss@xen-tools.org
https://xen-tools.org/mailman/listinfo/xen-tools-discuss

--===============3782814577050188526==--

