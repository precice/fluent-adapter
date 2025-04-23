#include "preciceC.h"
#include <stdio.h>


int mock_createParticipant_call_count = 0;
int mock_initialize_call_count = 0;
int mock_finalize_call_count = 0;

void precicec_createParticipant(const char* name,
                                const char* configFile,
                                int rank,
                                int size)
{
    mock_createParticipant_call_count++;
    printf("[Mock preCICE] createParticipant called with name=%s, config=%s, rank=%d, size=%d\n",
           name, configFile, rank, size);
}


void precicec_initialize(void)
{
    mock_initialize_call_count++;
    printf("[Mock preCICE] initialize called\n");
}

int precicec_isCouplingOngoing(void)
{
    return 1; 
}

double precicec_getMaxTimeStepSize(void)
{
    printf("[Mock preCICE] getMaxTimeStepSize called\n");
    return 0.01;
}



int precicec_requiresInitialData(void)
{
    printf("[Mock preCICE] precicec_requiresInitialData called\n");
    return 1; 
}

void precicec_advance(double dt)
{
    printf("[Mock preCICE] advance called with dt=%f\n", dt);
}

void precicec_finalize(void)
{
    mock_finalize_call_count++;
    printf("[Mock preCICE] finalize called\n");
}

void precicec_writeData(const char* meshName,
                        const char* dataName,
                        int size,
                        int* indices,
                        double* data)
{
    printf("[Mock preCICE] writeData called (mesh=%s, data=%s, size=%d)\n",
           meshName, dataName, size);
}

void precicec_readData(const char* meshName,
                       const char* dataName,
                       int size,
                       int* indices,
                       double dt,
                       double* outData)
{
    printf("[Mock preCICE] readData called (mesh=%s, data=%s, size=%d, dt=%f)\n",
           meshName, dataName, size, dt);

    for (int i = 0; i < size; i++){
        outData[i] = 0.0; 
    }
}
void precicec_setMeshVertices(const char *meshName, int size, const double *coordinates, int *ids)
{
    printf("[Mock preCICE] setMeshVertices called (mesh=%s, size=%d)\n",
           meshName, size);
}
int precicec_getDataDimensions(const char* meshName, const char* dataName)
{

    printf("[Mock preCICE] precicec_getDataDimensions called (%s, %s)\n",
           meshName, dataName);
    return 2;
}