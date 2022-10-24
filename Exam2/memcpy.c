char buf1[] = { 0, 1, 2, 3 };
char buf2[4] = { 0, 0, 0, 0 };
printf( "Before\n" );
for (i=0; i<4; i++) {
printf( "buf1[i] = %1d, buf2[i] = %1d\n", 
(int)buf1[i], (int) buf2[i] );
}   
memcpy( buf2, buf1, 4 ); // Copy the buffers
printf( "After\n" );
for (i=0; i<4; i++) {
printf( "buf1[i] = %1d, buf2[i] = %1d\n", 
(int) buf1[i], (int) buf2[i] );