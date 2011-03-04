// Quine-McCluskey minimization
//
// Patrick Mealey
// Joseph Gebhard

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string.h>

using namespace std;

struct Term{
    char* bits;
    bool essential;
    int decimal;
    Term(int sz = 4) : essential(false), decimal(0) {
        bits = new char[sz+1]; 
        memset(bits, '0', sz); 
        bits[sz] = 0;
    }
    ~Term() {delete bits;}
    Term( const Term& other ) :
        essential(other.essential) { 
            bits = new char[strlen(other.bits)];
            strcpy(bits, other.bits);
        }
};

int main(int argc, char** argv){
    // get input filename
    if (argc < 2){
        printf("No input file specified\n");
        return 1;
    }

    fstream infile(argv[1]);
    if (infile.fail()){
        printf("Error opening input file\n");
        return 2;
    }

    // read file header
    int numVars = 0;
    int numTerms = 0;

    infile >> numVars >> numTerms;

    printf("Got numVars: %d, numTerms:%d\n", numVars, numTerms);

    std::vector<Term> terms;
    // begin reading in terms
    for (int i=0; i < numTerms; ++i){
        char bit = 0;
        int t = 0;
        Term term(numVars);
        for (int k=numVars-1; k >= 0; --k){
            infile >> bit;
            if (bit == '1'){
                t += (int)pow(2.0, (double)k);
            } else if (bit != '0'){
                printf("Unexpected character in input t: %c\n", bit);
                return 3;
            }
            term.bits[k] = bit;
            // add the term to the list
            terms.push_back(term);
        }
        infile >> bit;
        printf("Got t: %d : %c\n", t, bit);
    }
}
