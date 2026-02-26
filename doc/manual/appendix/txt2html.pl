#!/usr/bin/perl

$name = $ARGV[0];

print <<EOF;
<html><head><title>MAT manual appendix: $name</title></head>
<body>
<!--autolink--><!--/autolink-->
<h1>Appendix: $name</h1>
<pre>
EOF
while (<>) {
  s/&/&amp;/g; s/</&lt;/g; s/>/&gt;/g;
  s/^([^#\n]+)/<font color="#CC0000">\1<\/font>/;
  s/^$/<\/pre><pre>\n/;
  print;
}
print <<EOF;
</pre>
<!--autolink--><!--/autolink-->
</body></html>
EOF

