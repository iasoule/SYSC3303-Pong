#if 0
    for(;;){
       int result;
       testfds = readfds;
       struct timeval timeout = {.tv_sec = 2*60, .tv_usec = 500000 };
	    result = select(FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0, &timeout);
       
       if(result < 1) {
	        if(result == 0){ printf("Waiting for connections ...\n"); continue;}	//timeout
	      errx(EXIT_FAILURE, "select():", strerror(errno));
	    }

       //have activity on one of the sockets 
       int fd;
       for(fd = 0; fd < FD_SETSIZE; fd++){
           if(FD_ISSET(fd, &testfds)){
            if(fd == serversock){
               unsigned int clientlen = sizeof(client_addr);
               clientsock = 
                 accept(serversock, (struct sockaddr *)&client_addr, 
                         (socklen_t *)&clientlen);
               if(clientsock == -1)
                  errx(EXIT_FAILURE, "accept():", strerror(errno));
               FD_SET(clientsock, &readfds);   
               //new client connected spawn a thread
               printf("%s connected\n",
                       inet_ntoa(client_addr.sin_addr));
               
               pthread_t worker_thread;
               if(pthread_create(&worker_thread, NULL, handle_client,
                                 (void *)clientsock))
                   errx(EXIT_FAILURE,  "pthread_create(): worker thread", strerror(errno));
            }
            else { //client closed connection
                int nread;
		        ioctl(fd, FIONREAD, &nread);
		        if(nread == 0){
			      close(fd);
			      FD_CLR(fd, &readfds);
			      printf("%s disconnected", inet_ntoa(client_addr.sin_a                         ddr));
		        }
               else { //client activity
#endif

