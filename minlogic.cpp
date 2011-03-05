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

    //intiallize an empty term struct
    Term(int sz = 4) : dontcare(false), essential(false), len(sz),copied(0) {
        bits = new char[sz+1]; 
        memset(bits, '0', sz); 
        bits[sz] = 0;
    }
    ~Term() {delete[] bits;}

    //create a clone of another term, incrementing the copied counter
    Term( const Term& other ) :
        dontcare(other.dontcare), essential(other.essential), len(other.len),copied(other.copied+1) { 
            bits = new char[len+1];
            strncpy(bits, other.bits, len+1);
            bits[len] = 0;
        }

    //overload the 'equals'comparaison operateer
    bool operator== (const Term& other){
        if (strncmp(bits, other.bits, len) == 0)
            if (dontcare == other.dontcare && essential == other.essential && len == other.len)
                return true;
        return false;
    }
};

//displays everything in the term vector one term per line
//
void printTerms(std::vector<Term*> terms){
    // for each term
    for (int i = 0; i < terms.size(); ++i){
        printf("Term: %s  %c%c\n", 
                terms[i]->bits, 
                terms[i]->dontcare ? 'd' : '1', 
                terms[i]->essential ? '*' : ' ');
    }
}

// TODO: this could be made more efficient... 
// we shouldn't need to compare every term to every other term.
// mergeTermsOnce returns a new vector of merged terms, 
// and modifies the terms in the input vector to mark those that are essential
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
            // if there was a single bit difference, merge
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
                delete newterms[k];
                newterms.erase(newterms.begin() + k);
                k--;
            }
        }
    }
    return newterms;
}

std::vector<Term*> mergeTerms(std::vector<Term*> terms){
    std::vector<Term*> merged;
    std::vector<Term*> lastmerged;

    std::vector<Term*> essential;

    // copy terms into merged
    std::vector<Term*>::iterator it;
    for (it = terms.begin(); it != terms.end(); ++it){
        Term* copy = new Term(*(*it));
        merged.push_back(copy);
    }

    bool done = false;
    // loop until nothing can be merged.
    while(done == false){
        // TODO: Fix memory leak -- delete the Terms in lastmerged first.
        // if i understood what you meant then i fixed the memory leak
        
        for (it = lastmerged.begin(); it != lastmerged.end(); ++it){
            delete (*it);	
        }

        lastmerged = merged;
        merged = mergeTermsOnce(lastmerged);

        // add anything that couldn't be merged to the essential vector
        for(int i = 0; i < lastmerged.size(); ++i){
            if (lastmerged[i]->essential){
                Term* newterm = new Term(*lastmerged[i]);
                essential.push_back(newterm);
            }
        }
        if (merged.size() == 0)
            done = true;
    }

    // clean up 
    for (it = lastmerged.begin(); it != lastmerged.end(); ++it){
        delete (*it);
    }

    return essential;
}

void printPIchart(bool** table, std::vector<Term*> terms, std::vector<Term*> implicants){
    char fmt[50];
    snprintf(fmt, 50, "%%%ds", implicants[0]->len + 2);
    printf(fmt, " ");
    for (int i = 0; i < terms.size(); ++i){
        printf(" %s ", terms[i]->bits);
    }
    printf("\n");

    for (int i = 0; i < implicants.size(); ++i){
        printf("%s  ", implicants[i]->bits);
        for(int k = 0; k < terms.size(); ++k){
            snprintf(fmt, 50, " %%%ds%%s%%%ds ", (terms[k]->len)/2-1, (terms[k]->len)/2);
            printf(fmt, " ", table[i][k] ? "x" : " ", " "); 
        }
        printf("\n");
    }
}
// Build the Prime Implicant Chart
bool** buildPI(std::vector<Term*> terms, std::vector<Term*> implicants){
    // create table
    bool** table = new bool*[implicants.size()];
    for(int i = 0; i < implicants.size(); ++i){
        table[i] = new bool[terms.size()];
        memset(table[i], 0, terms.size());
    }
    
    // fill it in
    for (int i = 0; i < implicants.size(); ++i){
        for(int k = 0; k < terms.size(); ++k){
            bool covers = true;
            for (int m = 0; m < implicants[i]->len; ++m){
                // if implicant bit == 0 and term bit isn't, 
                // this implicant does not cover the term
                if (implicants[i]->bits[m] == '0' && terms[k]->bits[m] != '0')
                    covers = false;
                // if implicant bit == 1 and term bit isn't, 
                // this implicant does not cover the term
                else if (implicants[i]->bits[m] == '1' && terms[k]->bits[m] != '1')
                    covers = false;
            }
            if (covers){
                table[i][k] = true;
            }
        }
    }
    return table;
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


    // clear essential flags
    for (int i = 0; i < terms.size(); ++i){
        terms[i]->essential = false;
    }
    for (int i = 0; i < merged.size(); ++i){
        merged[i]->essential = false;
    }
    
    // get the terms = 1
    // TODO: decide whether this should copy the term or just use the old one
    std::vector<Term*> ones;
    for (int i = 0; i < terms.size(); ++i){
        if (terms[i]->dontcare == false)
            ones.push_back(terms[i]);
    }
            
    // build prime implicant chart
    bool** pichart = buildPI(ones, merged);

    printPIchart(pichart, ones, merged);

    // clean up
    for (int i = 0; i < terms.size(); ++i){
        delete terms[i];
    }
    for (int i = 0; i < merged.size(); ++i){
        delete merged[i];
    }

    for (int i = 0; i < merged.size(); ++i){
        delete[] pichart[i];
    }
    delete[] pichart;
}
