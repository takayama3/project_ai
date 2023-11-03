#define pi 3.141592
#define e 2.718281


double random(double from, double to);
double random_int(int from, int to);


// typedef struct ActivationFunction {
// 	double (*f) (double x);
// 	double (*df) (double ideal, double output);
// } ActivationFunction;


double Sigmoid_f(double x);
double Sigmoid_df(double x);
// extern ActivationFunction Sigmoid;

double ReLU_f(double x);
double ReLU_df(double x);

void SoftMax_f(int length, double* x, double* y);
double SoftMax_df(double x);

double MSE_f(double ideal, double output);
double MSE_df(double ideal, double output);