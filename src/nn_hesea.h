#ifndef NN_HESEA_H
#define NN_HESEA_H

// Includes
#include "cryptocontext.h"
#include <bits/types/time_t.h>
#include <stdio.h>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <sys/time.h>
// Multi-processing
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <vector>

//! Qt modulus
#include <QObject>
#include <QWebChannel>
#include <QDebug>
#include <QMessageBox>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrl>


//! hesea
#define PROFILE

// #include "binfhecontext.h"
#include "hesea.h"
// #include <binfhecontext-ser.h>
using namespace lbcrypto;

// Defines
#define VERBOSE 1
#define STATISTICS true
#define WRITELATEX false
#define N_PROC 4

// Security constants
#define SECLEVEL 80
#define SECNOISE true
#define SECALPHA pow(2., -20)
#define SEC_PARAMS_STDDEV    pow(2., -30)
#define SEC_PARAMS_n  600                   ///  LweParams
#define SEC_PARAMS_N 1024                   /// TLweParams
#define SEC_PARAMS_k    1                   /// TLweParams
#define SEC_PARAMS_BK_STDDEV pow(2., -36)   /// TLweParams
#define SEC_PARAMS_BK_BASEBITS 10           /// TGswParams
#define SEC_PARAMS_BK_LENGTH    3           /// TGswParams
#define SEC_PARAMS_KS_STDDEV pow(2., -25)   /// Key Switching Params
#define SEC_PARAMS_KS_BASEBITS  1           /// Key Switching Params
#define SEC_PARAMS_KS_LENGTH   18           /// Key Switching Params

// The expected topology of the provided neural network is 256:30:10
#define NUM_NEURONS_LAYERS 3
#define NUM_NEURONS_INPUT  256
#define NUM_NEURONS_HIDDEN 30
#define NUM_NEURONS_OUTPUT 10

//! how many pics
#define CARD_TESTSET 80

// Files are expected in the executable's directory
#define PATH_TO_FILES       "buildotests/test/" //TODO FIXME!
#define FILE_TXT_IMG        "../weights-and-biases/txt_img_test.txt"
#define FILE_TXT_BIASES     "../weights-and-biases/txt_biases.txt"
#define FILE_TXT_WEIGHTS    "../weights-and-biases/txt_weights.txt"
#define FILE_TXT_LABELS     "../weights-and-biases/txt_labels.txt"
#define FILE_LATEX          "results_LaTeX.tex"
#define FILE_STATISTICS     "results_stats.txt"

// Tweak neural network
#define THRESHOLD_WEIGHTS  9
#define THRESHOLD_SCORE -100

#define MSG_SLOTS    700
#define TORUS_SLOTS  400


using namespace std;




class DINN_HESEA :public QObject
{
    // Q_OBJECT
    // Q_PROPERTY(QString enc_image_array MEMBER m_enc_image_array NOTIFY trans_image_from_C_to_js)
    // Q_PROPERTY(QString enc_scores_array MEMBER m_enc_scores_array NOTIFY trans_scores_from_C_to_js)
    // Q_PROPERTY(QString return_class MEMBER m_return_class NOTIFY trans_decrypt_from_C_to_js)
private:




public :
    string DATAFOLDER;
    CryptoContextImpl<DCRTPoly> m_cc;

    int* m_image;
    int m_p;

    QString enc_image;
    QString enc_score;

    LWEPrivateKey m_sk;
    
    
    DINN_HESEA();


    // int* get_image_from_string(const QString & );

    // string save_enc_data(int length,struct LweSample * );

    // struct LweSample* load_enc_data(const string ,struct LweSample *);

public slots:
    // const QString   encrypt(const QString &);

    // QString  net( const QString & );

    // QString decrypt( const QString & );
    void set_image(int *);

    QString encrypt(int* );

    QString net(QString );

    int decrypt(QString );

    QString save_as_qstring(vector<LWECiphertext>, bool is_enc_image);

    vector<LWECiphertext> load_from_qstring(QString, bool is_enc_image);


signals:
    void trans_image_from_C_to_js( QString  attr);
    void trans_scores_from_C_to_js(QString  attr);
    void trans_decrypt_from_C_to_js(QString  attr);



};

void deleteTensor(int*** tensor, int dim_mat, const int* dim_vec);
void deleteMatrix(int**  matrix, int dim_mat);
vector<string> split(const string &str, const string &pattern);
string readFileIntoString(string filename);
QString from_str_to_qstr(const string & str);
string from_qstr_to_str(const QString & qstr);



#endif