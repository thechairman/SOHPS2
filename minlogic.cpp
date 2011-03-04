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
    bool dontcare;
    bool essential;
    int len;
    int copied;
    Term(int sz = 4) : dontcare(false), essential(false), len(sz),copied(0) {
        bits = new char[sz+1]; 
        memset(bits, '0', sz); 
        bits[sz] = 0;
    }
    ~Term() {delete bits;}
    Term( const Term& other ) :
        dontcare(other.dontcare), essential(other.essential), len(other.len),copied(other.copied+1) { 
            bits = new char[len+1];
            strncpy(bits, other.bits, len+1);
            bits[len] = 0;
        }
    bool operator== (const Term& other){
        if (strncmp(bits, other.bits, len) == 0)
            if (dontcare == other.dontcare && essential == other.essential && len == other.len)
                return true;
        return false;
    }
};

void printTerms(std::vector<Term*> terms){
    // for each term
    for (int i = 0; i < terms.size(); ++i){
        printf("Term: %s  %c%c\n", 
                terms[i]->bits, 
                terms[i]->dontcare ? 'd' : '1', 
                terms[i]->essential ? '*' : ' ');
    }
}

// this could be made more efficient... 
// we shouldn't need to compare every term to every other term.
std::vector<Term*> mergeTermsOnce(std::vector<Term*> terms){
    std::vector<Term*> newterms;
    // mark all terms essential
    for(int i = 0; i < terms.size(); ++i){
        if (terms[i]->dontcare == false)
            terms[i]->essential = true;
    }
    // for each term
    for (int i = 0; i < terms.size(); ++i){
        // for each term after i
        for (int k = i+1; k < terms.size(); ++k){
            int bitdiff = -1;
            // for each bit
            for (int m = 0; m < terms[i]->len && m < terms[k]->len; ++m){
                // if bits differ
                if (terms[i]->bits[m] != terms[k]->bits[m]){
                    // if this is the second difference, break out
                    if (bitdiff != -1){
                        bitdiff = -1;
                        break;
                    // otherwise, mark it
                    } else {
                        bitdiff = m;
                    }
                }
            }
            // if there was a single bit difference
            if (bitdiff > -1){
                terms[i]->essential = false;
                terms[k]->essential = false;
                Term* new1 = new Term(*terms[i]);
                if (terms[i]->dontcare == false || terms[k]->dontcare == false)
                    new1->dontcare=false;
                new1->bits[bitdiff] = '-';
                newterms.push_back(new1);
            }
        }
    }
    // check for duplicates
    for (int i = 0; i < newterms.size(); ++i){
        for (int k = i+1; k < newterms.size(); ++k){
            // if 2 entries match, remove the latter one and continue
            if (strncmp(newterms[i]->bits, newterms[k]->bits, newterms[i]->len) == 0){
                newterms.erase(newterms.begin() + k);
                k--;
            }
        }
    }
    return newterms;
}

std::vector<Term*> mergeTerms(std::vector<Term*> terms){
    std::vector<Term*> merged = terms;
    std::vector<Term*> lastmerged;

    std::vector<Term*> essential;
    bool done = false;
    while(done == false){
        lastmerged = merged;
        merged = mergeTermsOnce(lastmerged);

        for(int i = 0; i < lastmerged.size(); ++i){
            if (lastmerged[i]->essential){
                Term* newterm = new Term(*lastmerged[i]);
                essential.push_back(newterm);
            }
        }
        if (merged.size() == 0)
            done = true;
    }
    return essential;
}

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

    std::vector<Term*> terms;
    // begin reading in terms
    for (int i=0; i < numTerms; ++i){
        char bit = 0;
        int t = 0;
        Term* term = new Term(numVars);
        for (int k=0; k < numVars; ++k){
            infile >> bit;
            if (bit == '1'){
                t += (int)pow(2.0, (double)(numVars-k-1));
            } else if (bit != '0'){
                printf("Unexpected character in input: %c\n", bit);
                return 3;
            }
            term->bits[k] = bit;
            // add the term to the list
        }
        infile >> bit;
        printf("Got t: %d : %c\n", t, bit);
        if (bit == 'd')
            term->dontcare = true;

        terms.push_back(term);
    }


    // merge terms
    std::vector<Term*> merged;
    merged = mergeTerms(terms);
    
    printf("Original Terms:\n");
    printTerms(terms);

    printf("Merged Terms:\n");
    printTerms(merged);
}
