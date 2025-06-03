#ifndef MOCK_PRECICEC_H
#define MOCK_PRECICEC_H

extern int mock_createParticipant_call_count;
extern int mock_initialize_call_count;
extern int mock_finalize_call_count;

void precicec_createParticipant(const char* name,
                                const char* configFile,
                                int rank,
                                int size);

void precicec_initialize(void);

int  precicec_isCouplingOngoing(void);

double precicec_getMaxTimeStepSize(void);

int  precicec_requiresInitialData(void);

void precicec_advance(double dt);

void precicec_finalize(void);

void precicec_writeData(const char* meshName,
                        const char* dataName,
                        int size,
                        int* indices,
                        double* data);

void precicec_readData(const char* meshName,
                       const char* dataName,
                       int size,
                       int* indices,
                       double dt,
                       double* outData);
void precicec_setMeshVertices(const char *meshName, int size, const double *coordinates, int *ids);
int precicec_getDataDimensions(const char *meshName, const char *dataName);

#endif /* MOCK_PRECICEC_H */
