#include "algo.hpp"


SomeOutput testfunc(std::vector<long double> inputs) {
    long double sum = 0;
    for (auto n : inputs) {
        sum += n;
    }
    return { 0, sum, o_PURE };
}
long double test_calculator(std::vector<long double> inputs) {
    long double kmax = 1;
    for (auto n : inputs) {
        kmax *= n;
    }
    return kmax;
}



    void DataType::calc_required_size() {
        required_size = 0;
        for (DT_Input& dti : encode_store) {
            required_size += (dti.history_amount);
        }
    }

    std::vector<float> get_so_values(const CompData& so) {
        std::vector<float> ors;
        for (auto x : so.cd) {
            switch (x.ot) {
            case o_PURE:
                ors.push_back(x.pure);
                break;
            case o_BINARY:
                ors.push_back(x.binary);
                break;
            }
        }
        return ors;
    }

    DataType::DataType(long double (*link)(std::vector<long double>), std::vector<DT_Input> encode) {
        DTFunction = link;
        temporary_inputs.resize(encode.size());
        encode_store = std::move(encode);

        calc_required_size(); //For sizecheck. Encode may have historical values more than 1 meaning inputs>encode
    }

    std::vector<DT_Input>* DataType::borrow_encode_storage() { //So that the main test function can gather necessary data.
        return &encode_store;
    }
    size_t DataType::req_size() {
        return required_size;
    }

    long double DataType::invoke(std::vector<long double> inputs) {
        /* Sizecheck 1*/
        if (inputs.size() != required_size) {
            std::cout << "[DataType " << this << "] sizecall -> inputs.size() != required_size [" << inputs.size() << " ? " << required_size << "]\n";
            return 0;
        }

        long double return_value = DTFunction(inputs);
        if (return_value == 0) {
            std::cout << "[DataType " << this << "] function was given " << inputs.size() << " and returned 0. Ensure?\n";
        }
        return return_value;
    }

#define max(a,b) a>b?a:b

    void Test::find_max_hist() {
        for (auto dt : dt_ptr_l) {
            auto ec = dt->borrow_encode_storage();
            for (int i = 0; i < ec->size(); i++) {
                for (const auto& f : ec[i]) {
                    max_hist = max(f.history_amount, max_hist);
                }
            }

        }
    }
    Test::Test(SomeOutput(*F)(std::vector<long double>), std::vector<DataType*> lf) {
        InputFunction = F;
        dt_ptr_l = lf;

        find_max_hist();
    }

    /* Calculations */
    double Test::avg_diff(const CompData& a, const CompData& b) {
        double accu = 0;

        double a_sum = std::accumulate(a.cd.begin(), a.cd.end(), 0.0f, [](double s, const SomeOutput& it) {
            if (it.ot == o_PURE) {
                return static_cast<double>(s + it.pure);
            }
            return (s + it.binary);
            });
        double b_sum = std::accumulate(b.cd.begin(), b.cd.end(), 0.0f, [](double s, const SomeOutput& it) {
            if (it.ot == o_PURE) {
                return static_cast<double>(s + it.pure);
            }
            return (s + it.binary);
            });
        std::cout << (abs(a_sum - b_sum));
        return (abs(a_sum - b_sum) / a.cd.size());
    }
    std::pair<double, double> Test::estimates(const CompData& a, const CompData& b) {
        double under = 0, over = 0;
        for (int j = 0; j < a.cd.size(); j++) {
            if (a.cd[j].pure < b.cd[j].pure) {
                under++;
            }
            else if (a.cd[j].pure > b.cd[j].pure) {
                over++;
            }
        }
        return { under/a.cd.size(), over/a.cd.size() };
    }
    double Test::accuracy(const CompData& a, const CompData& b, double range) {
        uint32_t sum = 0;
        for (int i = 0; i < a.cd.size(); i++) {
            switch (a.cd[i].ot) {
            case o_PURE:
                sum += (abs(a.cd[i].pure - b.cd[i].pure) < range);
                break;
            case o_BINARY:
                sum += (a.cd[i].binary == b.cd[i].binary);
            }
        }
        return (static_cast<double>(sum) / a.cd.size());
    }


    CompData Test::execute(PriceData pd, ExternData ed, CompData cd) {
        //Add a loop at some point. For now just test with 1.
        CompData outputs;
        for (int datapoint = 0; datapoint < cd.cd.size(); datapoint++) {
            std::vector<long double> final_inputs;
            for (DataType* dt : dt_ptr_l) {
                auto encoding = dt->borrow_encode_storage();
                size_t sz = dt->req_size();

                std::vector<long double> generated_inputs;

                //OMG! Why are we looping for encoding size and not req size? Encoding size is the number of datatypes!
                //Req size is the number of datapoints! Some datatypes may have >1 historical value, meaning we provide more than
                //Just the most recent value.

                auto it = encoding->begin();
                while (it != encoding->end()) {
                    auto encoding_value = *it;
                    std::vector<long double> to_insert;
                    switch (encoding_value.type) {
                    case OPEN:
                        for (int h = 0; h < encoding_value.history_amount; h++) {
                            to_insert.emplace_back(pd.open[datapoint - h]);
                        }
                        //to_insert = std::vector<long double>(encoding_value.history_amount, 1);
                        break;
                    case CLOSE:
                        for (int h = 0; h < encoding_value.history_amount; h++) {
                            to_insert.emplace_back(pd.close[datapoint - h]);
                        }
                        break;
                    case HIGH:
                        for (int h = 0; h < encoding_value.history_amount; h++) {
                            to_insert.emplace_back(pd.high[datapoint - h]);
                        }
                        break;
                    case LOW:
                        for (int h = 0; h < encoding_value.history_amount; h++) {
                            to_insert.emplace_back(pd.low[datapoint - h]);
                        }
                        break;
                    case EXTERNAL:
                        //generated_inputs[enc] = external_data[encoding_value.extern_type][enc]
                        for (int h = 0; h < encoding_value.history_amount; h++) {
                            to_insert.emplace_back(ed.externs[encoding_value.extern_type][datapoint - h]);
                        }
                        break;
                    }
                    generated_inputs.insert(generated_inputs.end(), to_insert.begin(), to_insert.end());

                    ++it;
                }
                final_inputs.push_back(dt->invoke(generated_inputs));
            }

            SomeOutput custom_prediction = InputFunction(final_inputs);
            outputs.cd.emplace_back(custom_prediction); //readability hahahaha
        }
        return outputs;
    }

