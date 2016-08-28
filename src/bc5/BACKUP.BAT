del c:\mswsrc%1.zip /p
call cleanall
pkzip -r -p -x*.obj -x*.zip -x*.rtf -x*.exe -x*.hlp -x*.res -x*.csm -x*.rws -x*.bak -xlogo16.doc c:\mswsrc%1.zip *.*
rem copy c:\mswsrc%1.zip a:
