#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include "mfind.h"
#include <sys/stat.h>

int main(int argc, char *argv[]) {

    //variables
    int flag;
    int nrthr = 0;
    int matchSet = false;
    threadArg arg;
    arg.directories = queue_empty();
    char flagtype;
    //queue* directories = queue_empty();
    //queue_setMemHandler(directories, free);

    //parse flags in argument
    //getopt verkar leta åt flaggor i argumentraden och lägger de i nån slags struktur
    //getint skapas...de är en int som pekar på n'ästa arg efter flaggorna
    while ((flag = getopt(argc, argv, "t:p:")) != -1) {
        switch (flag) {
            case 't':
                flagtype = *optarg;
                if(flagtype != 'd' && flagtype != 'f' && flagtype != 'l'){
                    fprintf(stderr, "Invalid parameters!\n"
                            "Usage: mfind [-t type] [-p nrthr] start1"
                            " [start2 ...] name\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                //atoi gör om t.ex 4 och 0 till 40 om man skrivit att man vill ha 40 trådar
                nrthr = atoi(optarg);
                if(nrthr == 0){
                    fprintf(stderr, "Invalid parameters!\n"
                            "Usage: mfind [-t type] [-p nrthr] start1"
                            " [start2 ...] name\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Invalid parameters!\n"
                        "Usage: mfind [-t type] [-p nrthr] start1"
                        " [start2 ...] name\n");
                exit(EXIT_FAILURE);
        }
    }
    //här är optind igen...lite skit för att få användaren att stoppa in rätt mängd argument
    //mycke är lånat från lorre
    if (argc - 1 == optind || optind >= argc){
        fprintf(stderr, "Invalid parameters!\n"
                "Usage: mfind [-t type] [-p nrthr] start1"
                " [start2 ...] name\n");
        exit(EXIT_FAILURE);
    }
    //funktionerna...under dom finns lite utskrifter om man vill undersöka flaggornas värde
    //men dom verka funka...tror man kan komma åt dom via optarg
    getDir(argc, argv, optind, &arg);
    search(&arg);//todo tråda iväg skiten nrthr ggroch snöra ihop sen
    queue_free(arg.directories);
    printf("\n%s", arg.filter);

    printf("\ntypeFlag: %c\nnrOfThreadFlag: %d ", flagtype, nrthr);

    printf("\nsearchFor: %s", arg.filter);


//    p_thread
//    2 = search(argv[1]);
//
//    joina trådar
//
//    skriv resultat

}
void search(threadArg* arg){
    //struktur där man kan spara ner folders
    DIR *dir;
    //2 till strukturer där man kan lagra ner filer för att få info om dom
    struct dirent *ent;
    struct stat st;
    char* path;
    char* filename;

    while(!queue_isEmpty(arg->directories)){//todo mutex på kön
        //spårutskrift för när den börjar bearbeta en mapp
        printf("\n*** Behandlar katalog: %s ***\n", (char *)queue_front(arg->directories));
        path = malloc(strlen(queue_front(arg->directories)) + 1);
        strcpy(path, queue_front(arg->directories));
        free(queue_front(arg->directories));
        queue_dequeue(arg->directories);
        //opendir öppnar en folder från sträng    //todo mutex av
        dir = opendir(path);
        if (dir) {
            //readdir dundrar igenom nästa fil i en folder och sparar i en struct dirent
            /* print all the files and directories within directory */
            while ((ent = readdir(dir)) != NULL) {
                filename = malloc(strlen(path) + strlen(ent->d_name) +2);
                strcpy(filename, path);
                //addera filnamnet på katalogpathen
                strcat(filename, ent->d_name);

                //lstat sparar ner info i en liknande struktur som dirent...fast den har lite annan info
                //kanske nån är överflödig men ja tror fan de krävs båda för
                //att få ut allt man vill ha
                if(lstat(filename, &st) != -1){ //todo lstat är röd? wierd IDE eller nån störd include som saknas
                }else{
                    fprintf(stderr,"%s", path);
                    perror("lstat: ");
                    fflush(stderr);
                }
                printf ("\nfilename: %s\n", ent->d_name);
                //S_ISLINK och dom andra kollar om filen är link, fil eller dir
                if(S_ISLNK(st.st_mode)) {
                    printf("Type: Symbolic link\n");  //todo skriva ut resultat korrekt
                    printf("Path: %s\n", filename);
                }
                else if(S_ISDIR(st.st_mode)){
                    printf ("Type: Directory\n");     //todo skriva ut resultat korrekt
                    printf("path: %s\n", filename);
                    //om de är en dir och inte är . eller .. så ska den addera till kön o bränna av en ny opendir
                    if(strcmp(ent->d_name, (char *) ".") != 0 &&
                            strcmp(ent->d_name, (char *) "..") != 0) {
                        strcat(filename, (char *) "/");
                        printf("***köar på: %s\n", filename); //todo mutex på kön
                        queue_enqueue(arg->directories, filename);//todo mutex av
                        continue;
                    }
                }
                else if(S_ISREG(st.st_mode)){
                    printf ("Type: File\n");        //todo skriva ut resultat korrekt
                    printf("path: %s\n", filename);
                }
                free(filename);
            }
            closedir (dir);
        } else {
            /* could not open directory */
            fprintf(stderr, "%s", path);
            perror("opendir:");
            fflush(stderr);
        }
        free(path);
    }
}

void getDir(int argc, char **argv, int nrArg, threadArg* arg){

    for(; nrArg < (argc-1); nrArg++){
        printf(">>>>Köar på: %s\n", argv[nrArg]);
        char* path = malloc(strlen(argv[nrArg])+1);
        strcpy(path, argv[nrArg]);
        queue_enqueue(arg->directories, path);
    }
    printf("\n ----------------   %s", argv[nrArg]);
    arg->filter = argv[nrArg];
}