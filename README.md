# pdfDownloaderPQI
This program accesses PM ftp site to download pdfs of printing job. The program reads the xml file in /work folder to update the jobID, which is used to look for pdfs belonging to the particular print job. Xml files are not opened (therefore xmlmessagehandler is not used at the present); the xml file name is read to update the jobID.

PDF files are deleted from the site after they are downloaded, but are backed up locally. 

PDF converter, converts each file to .lvt format. 
