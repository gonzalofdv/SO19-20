#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function
	//
	if(origin == NULL || destination == NULL)
		return -1;
	int c = 0;
	int numBytes = 0;

	while(!feof(origin) && numBytes < nBytes){
		c = getc(origin);
		putc(c, destination);
		numBytes++;
	}
	return numBytes;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has bee
 * kjhglkjhgfghjn properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	// Complete the function
	if(file == NULL){
		return NULL;
	}
	int tam = 0;
	char *cadena = NULL;
	int c;

	c = getc(file);
	while(c != '\0' && c != EOF){
		tam++;
		c = getc(file);
	}

	cadena = (char *)malloc(sizeof(char)*(tam + 1));

	if(cadena == NULL)
		return NULL;

	fseek(file, -(tam+1), SEEK_CUR);
	//fread(cadena, tam, 1, file);
	for(c=0; c < tam+1; ++c)
		cadena[c] = getc(file);
	
	return cadena;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	// Complete the function
	stHeaderEntry* cabecera = NULL;
	int nr_files = 0;
	//reserva de memoria
	fread(&nr_files, sizeof(int), 1,tarFile);
	cabecera = malloc(nr_files * sizeof(stHeaderEntry));

	if(cabecera == NULL){
		return NULL;
	}

	for(int i = 0; i < nr_files; i++){
		char* aux;
		aux = loadstr(tarFile);
		if(aux != NULL){
			cabecera[i].name = aux;
			fread(&cabecera[i].size, sizeof(unsigned int), 1, tarFile);
		}
		else{
			for(int j = 0; j < nr_files; j++){
				free(cabecera[i].name);
			}
			free(cabecera);
			fclose(tarFile);
			return NULL;
		}
	}

	(*nFiles) = nr_files;

	return cabecera;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	// Complete the function

	//FALTA MANEJAR TODOS LOS ERRORES (LOS NULL)

	FILE *mtar, *inputFile;
	stHeaderEntry *cabecera = NULL;
	int numBytes;

	mtar = fopen(tarName, "w");
	cabecera = malloc(sizeof(stHeaderEntry) * nFiles);

	if(cabecera == NULL)
		return EXIT_FAILURE;

	int tamFicheros = 0;
	int tamCabecera;

	for(int i = 0; i < nFiles; i++){
		tamCabecera = strlen(fileNames[i]) + 1;

		cabecera[i].name = malloc(tamCabecera);
		
		if(cabecera[i].name == NULL)
			return EXIT_FAILURE;

		strcpy(cabecera[i].name, fileNames[i]);

		tamFicheros += tamCabecera;
	}

	int offData = sizeof(int) + (nFiles * sizeof(unsigned int)) + tamFicheros;

	fseek(mtar, offData, SEEK_SET);

	for(int i = 0; i < nFiles; i++) {
		inputFile = fopen(fileNames[i], "r");

		if(inputFile == NULL){
			perror("Error abriendo input");
			return EXIT_FAILURE;
		}

		numBytes = copynFile(inputFile, mtar, INT_MAX);
		fclose(inputFile);
		cabecera[i].size = numBytes;

	}

	if(fseek(mtar, 0, SEEK_SET) == -1){
		perror("Error con fseek");
		return EXIT_FAILURE;
	}

	fwrite(&nFiles, sizeof(int), 1, mtar);

	for(int i = 0; i < nFiles; i++){
		fwrite(cabecera[i].name, strlen(fileNames[i])+1,1,mtar);
	//	fwrite("\n", sizeof(char), 1, mtar);
		//fwrite(strcat(&cabecera[i].name, '\0'), strlen(cabecera[i].name) + 1, 1, mtar); //comprobar si esto escribe dos veces \0 porque no sabemos si con cabecera[i].name seria suficiente
		fwrite(&cabecera[i].size, sizeof(int), 1, mtar); 
	//	fwrite("\n", sizeof(char),1,mtar);
		free(cabecera[i].name);

	}
/*
	for(int i = 0; i < nFiles; i++){
		free(cabecera[i].name);
		//free(&cabecera[i].size);
	}*/
	free(cabecera);
	fclose(mtar);

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	// Complete the function

	FILE * tarFile, *archivoSalida;

	int nFiles = 0;

	char *nombre;

	int wBytes;

	tarFile = fopen(tarName, "r");

	if(tarFile == NULL){
		return EXIT_FAILURE;
	}

	stHeaderEntry *cabecera = readHeader(tarFile, &nFiles);

	if(cabecera == NULL){
		return EXIT_FAILURE;
	}

	for(int i = 0; i < nFiles; i++){
		nombre = cabecera[i].name;

		archivoSalida = fopen(nombre, "w"); //que permisos dar?

		if((wBytes = copynFile(tarFile, archivoSalida, cabecera[i].size)) != cabecera[i].size){

			perror("Error escribiendo en el fichero");
			return(EXIT_FAILURE);
		}

		fclose(archivoSalida);

	}
	/*
	for(int i = 0; i < nFiles; i++){
		free(cabecera[i].name);
	}
	*/
	fclose(tarFile);
	free(nombre);
	free(cabecera);

	return EXIT_SUCCESS;
}


int removeFromTar(char remName[], char tarName[]){

	//remName = fichero a eliminar
	//tarName = tarball

	//Leer la cabecera del tarball

	FILE * tarFile;
	FILE * new;
	int nFiles;
	stHeaderEntry *cabecera = NULL;

	tarFile = fopen(tarName, "r");

	cabecera = readHeader(tarFile, &nFiles);

	//Crear un fichero nuevo a partir del nombre original añadiendo "_TMP_". Usar para ello:
		//strlen: calcula la longitud de una cadena de caracteres excluyendo '\0'
		//strcat: añade una cadena de caracteres al final de otra, para lo cual deberá de haber
		//espacio previamente

	int tamArray = strlen(tarName) + strlen("_TMP_") + 1;

	char * newName = malloc(tamArray);

	//newName = strcat(tarName, "_TMP_");
	strcpy(newName, tarName);
	strcat(newName, "_TMP_");


	int tamCabecera = 0;
	int tamFicheros = 0;

	//Calcular el tamaño de la nueva cabecera. Para saber qué fichero no debemos de tener en cuenta usaremos la siguiente función:
		//strcmp: compara dos cadenas de caracteres, devolviendo 0 si son iguales.

	for(int i = 0; i < nFiles; ++i){
		if(strcmp(cabecera[i].name, remName) != 0){
			tamCabecera = strlen(cabecera[i].name) + 1;
			tamFicheros += tamCabecera;
		}
	}

	int offData = sizeof(int)+((nFiles-1) * sizeof(unsigned int)) + tamFicheros;

	new = fopen(newName, "w");
	fseek(new, offData, SEEK_SET);

	//Copiar en el fichero nuevo los datos del tarball original de forma selectiva y en el mismo
	//orden que seguimos en la función createTar. No olvideos mover el puntero de lectura/escritura
	//para saltar la región de datos que no hay que copiar.
	
	for(int i = 0; i < nFiles; ++i){
		if(strcmp(cabecera[i].name, remName) != 0)
			copynFile(tarFile, new, cabecera[i].size);
		else
			fseek(tarFile, cabecera[i].size, SEEK_CUR);
	}

	fclose(tarFile);
	fseek(new, 0, SEEK_SET);
	tamCabecera = nFiles - 1;
	fwrite(&tamCabecera, sizeof(int), 1, new);

	for(int i = 0; i < nFiles; ++i) {
		if(strcmp(cabecera[i].name, remName) != 0){
			fwrite(cabecera[i].name, strlen(cabecera[i].name)+1, 1, new);
			fwrite(&cabecera[i].size, sizeof(unsigned int), 1, new);
			free(cabecera[i].name);
		}
	}

	//Por ultimo borramos el fichero original y renombramos el nuevo:
		//remove: borra un nombre del sistema de ficheros
		//rename: renombra el fichero.

	fclose(new);
	remove(tarName);
	rename(newName, tarName);

	return EXIT_SUCCESS;
}