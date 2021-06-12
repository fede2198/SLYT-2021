Per unire i file di output di tdc e qdc, seguire i seguenti passaggi:
1)scaricare sul pc i file .dat di qdc e tdc
2)metterli in una stessa cartella ed eseguire il seguente comando:
paste nomefile_qdc nomefile_tdc > nomefile
3)dentro nomefile si trova l'output unificato, MA bisogna togliere il cancelletto che si trova all'inizio dei TCH
4)entrare su root ed eseguire il seguente comando, controllando che nella cartella ci sia il file root.cpp:
root ASCII2TTree("nomefile","nomefile.root",32,16)
5)Nella cartella si trova il file nomefile.root che contiene tutti i dati messi correttamente
