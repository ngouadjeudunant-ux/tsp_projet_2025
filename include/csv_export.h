#ifndef CSV_EXPORT_H
#define CSV_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif

int export_tour_csv(const char *filename, const int *tour, int n);

int export_summary_csv(const char *filename, const char *instance_name, const char *method,
                       double duration_sec, double cost, const int *tour, int n);
                       
#ifdef __cplusplus
}
#endif

#endif
