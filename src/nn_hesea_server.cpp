#include "nn_hesea_server.h"
#include "cryptocontext.h"
#include "lwecore.h"
#include "math/backend.h"
#include "utils/inttypes.h"
#include "utils/serial.h"
#include "utils/sertype.h"
#include <fstream>
#include <ios>
#include <string>
#include <vector>




DINN_HESEA_SERVER::DINN_HESEA_SERVER(){
    //! binfhecontext strat
    m_cc = CryptoContextImpl<DCRTPoly>();

    m_p = 512;

    m_cc.Generate_Default_params();
    int q = m_cc.HESea_GetParams()->GetLWEParams()->Getq().ConvertToInt();

    // Serialize keypoint data
    DATAFOLDER = "."; 
};




string DINN_HESEA_SERVER::net(string get_qstring){

    vector<LWECiphertext> enc_image;
    enc_image = load_from_string(get_qstring, true);

    NativeInteger q = enc_image[0]->GetA().GetModulus();
    

    // Network specific
    const int num_wire_layers = NUM_NEURONS_LAYERS - 1;
    const int num_neuron_layers = NUM_NEURONS_LAYERS;
    const int num_neurons_in = NUM_NEURONS_INPUT;
    const int num_neurons_hidden = NUM_NEURONS_HIDDEN;
    const int num_neurons_out = NUM_NEURONS_OUTPUT;

    // Vector of number of neurons in layer_in, layer_H1, layer_H2, ..., layer_Hd, layer_out;
    const int topology[num_neuron_layers] = {num_neurons_in, num_neurons_hidden, num_neurons_out};


    const bool clamp_biases  = false;
    const bool clamp_weights = false;

    const bool statistics        = STATISTICS;
    const bool writeLaTeX_result = WRITELATEX;

    const int threshold_biases  = THRESHOLD_WEIGHTS;
    const int threshold_weights = THRESHOLD_WEIGHTS;


    // Program the wheel to value(s) after Bootstrapping
    // const Torus32 mu_boot = modSwitchToTorus32(1, space_after_bs);

    // Huge arrays
    int*** weights = new int**[num_wire_layers];  // allocate and fill matrices holding the weights
    NativeInteger*** weights_1 = new NativeInteger**[num_wire_layers];
    int ** biases  = new int* [num_wire_layers];  // allocate and fill vectors holding the biases

    // Temporary variables
    string line;
    int el, l;
    int num_neurons_current_layer_in, num_neurons_current_layer_out;

    if (VERBOSE) cout << "IMPORT WEIGHTS, BIASES FROM FILES" << endl;
    if (VERBOSE) cout << "Reading images (regardless of dimension) from " << FILE_TXT_IMG << endl;


    if (VERBOSE) cout << "Reading weights from " << FILE_TXT_WEIGHTS << endl;
    ifstream file_weights(FILE_TXT_WEIGHTS);

    num_neurons_current_layer_out = topology[0];
    for (l=0; l<num_wire_layers; ++l)
    {
        num_neurons_current_layer_in = num_neurons_current_layer_out;
        num_neurons_current_layer_out = topology[l+1];

        weights[l] = new int*[num_neurons_current_layer_in];
        weights_1[l] = new NativeInteger*[num_neurons_current_layer_in];
        for (int i = 0; i<num_neurons_current_layer_in; ++i)
        {
            weights[l][i] = new int[num_neurons_current_layer_out];
            weights_1[l][i] = new NativeInteger[num_neurons_current_layer_out];
            for (int j=0; j<num_neurons_current_layer_out; ++j)
            {
                getline(file_weights, line);
                el = stoi(line);
                if (clamp_weights)
                {
                    if (el < -threshold_weights)
                        el = -threshold_weights;
                    else if (el > threshold_weights)
                        el = threshold_weights;
                    // else, nothing as it holds that: -threshold_weights < el < threshold_weights
                }
                weights[l][i][j] = el;
                // NativeInteger a = NativeInteger((el+p)%p);
                weights_1[l][i][j] = NativeInteger((el+q)%q);
            }
        }
    }
    file_weights.close();


    if (VERBOSE) cout << "Reading biases from " << FILE_TXT_BIASES << endl;
    ifstream file_biases(FILE_TXT_BIASES);

    num_neurons_current_layer_out = topology[0];
    for (l=0; l<num_wire_layers; ++l)
    {
        num_neurons_current_layer_in = num_neurons_current_layer_out;
        num_neurons_current_layer_out = topology[l+1];

        biases [l] = new int [num_neurons_current_layer_out];
        for (int j=0; j<num_neurons_current_layer_out; ++j)
        {
            getline(file_biases, line);
            el = stoi(line);
            if (clamp_biases)
            {
                if (el < -threshold_biases)
                    el = -threshold_biases;
                else if (el > threshold_biases)
                    el = threshold_biases;
                // else, nothing as it holds that: -threshold_biases < el < threshold_biases
            }
            biases[l][j] = el;
        }
    }
    file_biases.close();


    if (VERBOSE) cout << "Import done. END OF IMPORT" << endl;


    int** weight_layer;
    int * bias;
    int w0;


    NativeInteger w_1;
    vector<LWECiphertext> multi_sum_1;
    vector<LWECiphertext> bootstrapped;

    int class_enc = 0;
    LWEPlaintext score_1;

    num_neurons_current_layer_out= topology[0];
    num_neurons_current_layer_in = num_neurons_current_layer_out;


    // ========  FIRST LAYER(S)  ========

    for (l=0; l<num_wire_layers - 1 ; ++l)     // Note: num_wire_layers - 1 iterations; last one is special. Access weights from level l to l+1.
    {
        // To be generic...
        num_neurons_current_layer_in = num_neurons_current_layer_out;
        num_neurons_current_layer_out= topology[l+1];
        bias = biases[l];
        weight_layer = weights[l];
        for (int j=0; j<num_neurons_current_layer_out; ++j)
        {
            w0 = bias[j];
            auto ct = m_cc.HESea_TraivlEncrypt((w0 + m_p) % m_p, m_p);
            multi_sum_1.push_back(ct);
            for (int i=0; i<num_neurons_current_layer_in; ++i)
            {

                //! compute in ciphertext
                w_1 = weights_1[l][i][j];
                const NativeVector temp_A = enc_image[i]->GetA().ModMul(w_1);
                auto temp_B = enc_image[i]->GetB().ModMul(w_1, q);
                multi_sum_1[j]->SetA(temp_A.ModAdd(multi_sum_1[j]->GetA()));
                multi_sum_1[j]->SetB(temp_B.ModAdd(multi_sum_1[j]->GetB(), q));
            }
        }
    }

    for (int j=0; j<num_neurons_current_layer_out; ++j)
    {
        //! signfunc by our method
        auto ct_sign = m_cc.HESea_MyEvalSigndFunc(multi_sum_1[j], m_p);
        bootstrapped.push_back(ct_sign);
    }

    
    
    //! clear the vector mult_sum_1 to compute the next layer 
    multi_sum_1.clear();
    // ========  LAST (SECOND) LAYER  ========
    bias = biases[l];
    weight_layer = weights[l];
    l++;
    num_neurons_current_layer_in = num_neurons_current_layer_out;
    num_neurons_current_layer_out= topology[l]; // l == L = 2

    for (int j=0; j<num_neurons_current_layer_out; ++j)
    {
        w0 = bias[j];
        auto ct = m_cc.HESea_TraivlEncrypt((w0 + m_p) % m_p, m_p);
        multi_sum_1.push_back(ct);                 

        for (int i=0; i<num_neurons_current_layer_in; ++i)
        {
            w_1 = weights_1[l-1][i][j]; 

            // process the encrypted data 
            auto temp_A = bootstrapped[i]->GetA().ModMul(w_1);
            auto temp_B = bootstrapped[i]->GetB().ModMulFast(w_1, bootstrapped[i]->GetA().GetModulus());
            multi_sum_1[j]->SetA(temp_A.ModAdd(multi_sum_1[j]->GetA()));
            multi_sum_1[j]->SetB(temp_B.ModAdd(multi_sum_1[j]->GetB(),multi_sum_1[j]->GetA().GetModulus()));

        }
    }

    enc_image.clear();
    bootstrapped.clear();

    // free memory
    // delete_gate_bootstrapping_secret_keyset(secret);
    // delete_gate_bootstrapping_parameters(params);

    deleteTensor(weights,num_wire_layers, topology);
    deleteMatrix(biases, num_wire_layers);

    // serialize ciphertext
    string res = save_as_string(multi_sum_1, false);

    return res;




}



string DINN_HESEA_SERVER::save_as_string(vector<LWECiphertext> enc_data, bool is_enc_image){
    int n = is_enc_image? 256 : 10;
    // serialize ciphertext
    ofstream temp;
    temp.open((DATAFOLDER + "/ct.txt"), ios_base::trunc | ios_base::binary);
    for (int i = 0; i < n; ++i){
        Serial::Serialize(enc_data[i], temp, SerType::BINARY);
        temp<<"_+{}|;/.,";
        // if (!Serial::SerializeToFile(DATAFOLDER + "/ct" + to_string(i) + ".txt", m_enc_image[i], SerType::BINARY)) {
        //     cerr << "Error serializing ct1" << endl;
        // }
    }
    if(is_enc_image){
        temp<<"+=-_!@#$%^&*()";
        Serial::Serialize(m_cc.HESea_GetRefreshKey(), temp, SerType::BINARY);
        temp<<"+=-_!@#$%^&*()";
        Serial::Serialize(m_cc.HESea_GetSwitchKey(), temp, SerType::BINARY);
        temp<<"+=-_!@#$%^&*()";   
        temp.close();
        cout << "ciphertext and BootstrpKey has been serialized." << std::endl;
    }
    else {
        temp.close();
        cout << "enc_score has been serialized." << std::endl;
    }
    string cipherbk = readFileIntoString(DATAFOLDER + "/ct.txt");

    return cipherbk;

}


vector<LWECiphertext> DINN_HESEA_SERVER::load_from_string(string enc_data, bool is_enc_image){
    int n = is_enc_image? 256 : 10;
    cout << "begin to deserialize from string " << endl;
    ofstream fout;
    ifstream fin;
    vector<LWECiphertext> enc_cipher;
    vector<string> cipher;
    //! deseriealize cc
    if(is_enc_image){
        vector<string> cipher_b_k = split(enc_data, "+=-_!@#$%^&*()");

        // deserializing the refreshing and switching keys (for bootstrapping)
        fout.open(DATAFOLDER + "/temp.txt",ios_base::out | ios_base::binary | ios_base::trunc);
        fout << cipher_b_k[1];
        fout.close();
        fin.open(DATAFOLDER + "/temp.txt",ios_base::in | ios_base::binary);
        std::shared_ptr<RingGSWBTKey> refreshKey;
        Serial::Deserialize(refreshKey, fin, SerType::BINARY);
        fin.close();

        fout.open(DATAFOLDER + "/temp.txt",ios_base::out | ios_base::binary | ios_base::trunc);
        fout << cipher_b_k[2];
        fout.close();
        fin.open(DATAFOLDER + "/temp.txt",ios_base::in | ios_base::binary);
        std::shared_ptr<LWESwitchingKey> ksKey;
        Serial::Deserialize(ksKey, fin, SerType::BINARY);
        fin.close();

        // Loading the keys in the cryptocontext
        m_cc.HESea_BTKeyLoad({refreshKey, ksKey});

        //! DeserializeFromFile enc_image
        cipher = split(cipher_b_k[0], "_+{}|;/.,");
    }
    else {
        cipher = split(enc_data, "_+{}|;/.,");
    }

    for (int i = 0; i < n; ++i){
        LWECiphertext ct;
        fout.open(DATAFOLDER + "/temp.txt",ios_base::out | ios_base::binary | ios_base::trunc);
        fout << cipher[i];
        fout.close();
        fin.open(DATAFOLDER + "/temp.txt",ios_base::in | ios_base::binary);
        Serial::Deserialize(ct, fin, SerType::BINARY);
        enc_cipher.push_back(ct);
        fin.close();
    }

    cout << "deserializing has finished " << endl;
    //! finish

    return enc_cipher;

}

void deleteTensor(int*** tensor, int dim_tensor, const int* dim_vec)
{
    int** matrix;
    int dim_mat;
    for (int i=0; i<dim_tensor; ++i)
    {
        matrix =  tensor[i];
        dim_mat = dim_vec[i];
        deleteMatrix(matrix, dim_mat);
    }
    delete[] tensor;
}


void deleteMatrix(int** matrix, int dim_mat)
{
    for (int i=0; i<dim_mat; ++i)
    {
        delete[] matrix[i];
    }
    delete[] matrix;
}


vector<string> split(const string &str, const string &pattern)
{
    vector<string> res;
    if(str == "")
        return res;
    string strs = str; /*+ pattern;*/
    size_t pos = strs.find(pattern);
    while(pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos+pattern.size(), strs.size());
        pos = strs.find(pattern);
    }
    return res;
}

string readFileIntoString(string filename)
{
    ifstream ifile(filename);
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
    {
        buf.put(ch);
    }
    return buf.str();
}


