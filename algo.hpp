#include <vector>
#include <string>
#include <iostream>
#include <numeric>


typedef enum {
    OPEN, //open price
    CLOSE, //close price
    HIGH, //high price
    LOW, //low price
    EXTERNAL //external calculated value
} DT_InputType;

typedef struct {
    DT_InputType type;
    int history_amount; //default is 1- 1 value.
    int extern_type; //only needed if it's of type extern. used to determine which external dataset to pull from!
} DT_Input;
//Dynamic struct so you can choose what you predict
typedef enum {
    o_BINARY, //1 for up, 0 for down
    o_PURE, //price
} OutputType;

typedef struct {
    short binary;
    long double pure;
    OutputType ot;
} SomeOutput;
typedef struct {
    std::vector<SomeOutput> cd;
} CompData;

typedef struct {
    std::vector<long double> open, close, high, low;
} PriceData;
typedef struct {
    std::vector<std::vector<long double>> externs;
} ExternData;

std::vector<float> get_so_values(const CompData&);

class DataType {
private:
    long double (*DTFunction) (std::vector<long double>);
    std::vector<long double> temporary_inputs; //modified on every invoke. calculated values that are sent to the function ptr
    std::vector<DT_Input> encode_store;
    size_t required_size;

    void calc_required_size();
public:


    DataType(long double (*link)(std::vector<long double>), std::vector<DT_Input> encode);

    std::vector<DT_Input>* borrow_encode_storage();  //So that the main test function can gather necessary data.
       
    size_t req_size();

    long double invoke(std::vector<long double> inputs);
};
#define max(a,b) a>b?a:b
class Test {
private:
    SomeOutput(*InputFunction) (std::vector<long double>); //callback
    std::vector<DataType*> dt_ptr_l;
    int max_hist = 1;

    void find_max_hist();
public:
    Test(SomeOutput(*F)(std::vector<long double>), std::vector<DataType*> lf);

    /* Calculations */
    double avg_diff(const CompData& a, const CompData& b);
    std::pair<double, double> estimates(const CompData& a, const CompData& b);
    double accuracy(const CompData& a, const CompData& b, double range);

    CompData execute(PriceData pd, ExternData ed, CompData cd);
        
    
};