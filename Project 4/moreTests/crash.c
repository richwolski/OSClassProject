main()
{
   int fd;
   int err;
 
   fd = 17723;
   err = write(fd,"FAIL",strlen("FAIL"));
   close(fd);
   exit(0);
}