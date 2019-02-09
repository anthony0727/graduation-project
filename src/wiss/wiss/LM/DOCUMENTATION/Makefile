
nothing: 
		echo 'do what?'

rep:	report
		ditroff -P7a -me  -tbl -t report > report.dvi
		xproof report.dvi

inst:	install
		ditroff -P7a -me -tbl -t xoct.install > xoct.install.dvi
		xproof xoct.install.dvi

ti:	title
		ditroff -P7a -me -tbl -t title > title.dvi
		xproof title.dvi
pr:	
		lpr -n -P7a report.dvi
