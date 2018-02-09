# Appliations for Patmos and T-CREST

Small example programs (single file) live in folder c.
Larger applications shall live in their own folder under
apps and use a single main file. If additional files are needed, they can be added under <app_name>/include. 

Multiple main files reside in <app_name>/ but only one of them will be used at a time.

To run an app go to t-crest/patmos and write:

make app APP=<app_name> MAIN=<main_file_name>
