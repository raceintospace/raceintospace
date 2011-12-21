#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

void print_escaped_string(const char * string, int max_length)
{
    int i;
    for (i = 0; i < max_length; i ++) {
        char c = string[i];
        switch (c) {
            case 0:
                // NUL = end-of-string
                return;
            
            case '\\': fwrite("\\\\", 1, 4, stdout); break;
            case '"': fwrite("\\\"", 1, 4, stdout); break;

            default:
                if (c >= ' ' && c <= '~')
                    putc(c, stdout);
                else
                    printf("\\x%02x", c);
        }
    }
}

#define RecordStart() { printf("  {\n"); }
#define RecordEnd() { printf("  },\n"); }
#define Number(name) { printf("    ." #name " = %i,\n", (int)record.name); }
#define NumberArray(name) do { \
        int i; \
        printf("    ." #name " = {"); \
        for (i = 0; i < (sizeof(record.name) / sizeof(record.name[0])); i++) { \
            printf("%s %i", (i == 0 ? "" : ","), (int)record.name[i]); \
        } \
        printf(" },\n"); \
    } while(0)
#define String(name) { \
    printf("    ." #name " = \""); \
    print_escaped_string(record.name, sizeof(record.name)); \
    printf("\",\n", record.name); \
    }

void write_mission(FILE * fp)
{
    struct {
        char Index;       /**< Mission Index Number */
        char Name[50];    /**< Name of Mission */
        char Abbr[24];    /**< Name of Mission Abbreviated */
        char Code[62];    /**< Mission Coding String */
        char Alt[36];     /**< Alternate Sequence */
        char AltD[36];    /**< Alternate Sequence for Deaths (Jt Missions Only) */
        char Days,        /**< Duration Level */
             Dur,              /**< Duration Mission */
             Doc,              /**< Docking Mission */
             EVA,              /**< EVA Mission */
             LM,               /**< LM Mission */
             Jt,               /**< Joint Mission */
             Lun,              /**< Lunar Mission */
             mEq,              /**< Minimum Equipment */
             mCrew;            /**< Useful for Morgans Code */
        uint8_t mVab[2];  /**< Hardware Requirements */
        char PCat[5],     /**< Prestige Category List */
             LMAd;             /**< LM Addition Points */
    } __attribute__((packed)) record;

    printf("struct mStr missions[] = {\n");
    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        Number(Index);
        String(Name);
        String(Abbr);
        String(Code);
        NumberArray(Alt);
        NumberArray(AltD);
        Number(Days);
        Number(Dur);
        Number(Doc);
        Number(EVA);
        Number(LM);
        Number(Jt);
        Number(Lun);
        Number(mEq);
        Number(mCrew);
        NumberArray(mVab);
        NumberArray(PCat);
        Number(LMAd);
        RecordEnd();
    }
    printf("};\n\n");
}

void write_men(FILE * fp, const char * name)
{
    struct ManPool {
        char Name[14], Sex, Cap, LM, EVA, Docking, Endurance;
    } __attribute__((packed)) record;
    
    printf("struct ManPool %s[] = {\n", name);
    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        String(Name);
        Number(Sex);
        Number(Cap);
        Number(LM);
        Number(EVA);
        Number(Docking);
        Number(Endurance);
        RecordEnd();
    }
    printf("};\n\n");
}

void write_historical_men(FILE * fp) {
    write_men(fp, "historical_men");
}

void write_custom_men(FILE * fp) {
    write_men(fp, "custom_men");
}

void decode(const char * dir, const char * filename, void(*function)(FILE *))
{
    char full_path[512];
    FILE * fp;
    
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, filename);
    fp = fopen(full_path, "rb");

    if (!fp) {
        char error[600];
        snprintf(error, sizeof(error), "unable to open %s", full_path);
        perror(error);
        exit(1);
    }
    
    function(fp);
    
    fclose(fp);
}

int main(int argc, char ** argv)
{
    FILE * fp;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s path/to/gamedata\n", argv[0]);
        return 1;
    }
    
    decode(argv[1], "mission.dat", write_mission);

    decode(argv[1], "crew.dat", write_historical_men);
    decode(argv[1], "user.dat", write_custom_men);

    return 0;
}