// Quine-McCluskey minimization
//
// Patrick Mealey
// Joseph Gebhard

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
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
    ~Term() {
        delete[] bits;
        //printf("deleting term which has been copied %d times.\n", copied);
    }

    //create a clone of another term, incrementing the copied counter
    Term( const Term& other ) :
        dontcare(other.dontcare), essential(other.essential), len(other.len),copied(other.copied+1) { 
            bits = new char[len+1];
            strncpy(bits, other.bits, len+1);
            bits[len] = 0;
        }

    //overload the 'equals' comparaison operateer
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
    /*
    if (implicants.size() == 0){
        printf("No prime implicants for table.\n");
        return;
    }
    if (terms.size() == 0){
        printf("No terms for table.\n");
        return;
    }
    char fmt[50];
    snprintf(fmt, 50, "%%%ds", implicants[0]->len + 2);
    printf(fmt, " ");
    for (int i = 0; i < terms.size(); ++i){
        printf(" %s ", terms[i]->bits);
    }
    printf("\n");

    for (int i = 0; i < implicants.size(); ++i){
        printf("%s%c ", implicants[i]->bits, implicants[i]->essential ? '*' : ' ');
        for(int k = 0; k < terms.size(); ++k){
            snprintf(fmt, 50, " %%%ds%%s%%%ds ", (terms[k]->len)/2-1, (terms[k]->len)/2);
            printf(fmt, " ", table[i][k] ? "x" : " ", " "); 
        }
        printf("\n");
    }
    */
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

std::vector<Term*> findMin(bool** table, std::vector<Term*> terms, std::vector<Term*> implicants){
    bool** table_ = table;
    std::vector<Term*> imps_ = implicants;
    std::vector<Term*> terms_ = terms;

    // TODO: not sure whether this loop is useful or not...
    //          Apparently it breaks things...
    bool shrunk = true;
    //while(shrunk){
    //    shrunk = false;
        // if a column has only one 'x', then that implicant is essential.
        for (int k = 0; k < terms_.size(); ++k){
            int x = -1;
            for (int i = 0; i < imps_.size(); ++i){
                if (table[i][k] && x >= 0){
                    x = -1;
                    break;
                } else if (table[i][k]){
                    x = i;
                }
            }
            if (x >= 0){
                // implicant x is essential
                imps_[x]->essential = true;
                shrunk = true;
            }
        }

        // check if we have any non-essential implicants
        bool foundnes = false;
        for (int i = 0; i < implicants.size(); ++i){
            if (implicants[i]->essential == false){
                foundnes = true;
            }
        }
        if (foundnes == false){
            std::vector<Term*> ret;
            for (int i = 0; i < implicants.size(); ++i){
                if (implicants[i]->essential)
                    ret.push_back(implicants[i]);
            }
            sort(ret.begin(), ret.end());
            return ret;
        }

        // make smaller table without essential implicants and their covered terms
        imps_.clear();
        terms_ = terms;

        for (int i = 0; i < implicants.size(); ++i){
            if (implicants[i]->essential == false){
                imps_.push_back(implicants[i]);

            } else {
                for (int k = 0; k < terms.size(); ++k){
                    if (table[i][k]){
                        for (int m = 0; m < terms_.size(); ++m){
                            if (*(terms[k]) == *(terms_[m])){
                                terms_.erase(terms_.begin() + m);
                                m--;
                            }
                        }
                    }
                }
            }
        }

        table_ = buildPI(terms_, imps_);

        printPIchart(table_, terms_, imps_);
    //}

    // Implement Petrick's method to reduce prime implicant table

    // Begin by forming a POS from the table
    std::vector< std::vector< std::vector<Term*> > > pos;
    for (int i = 0; i < terms_.size(); ++i){
        for (int k = 0; k < imps_.size(); ++k){
            
        }
    }


    // find the groups that cover with the fewest literals.
    // (the most dashes)
    std::vector<Term*> mostdash;
    int max = 0;
    for (int i = 0; i < fgroups.size(); ++i){
        int dashes = 0;
        for (int k = 0; k < fgroups[i].size(); ++k){
            for (int m = 0; m < fgroups[i][k]->len; ++m){
                if (fgroups[i][k]->bits[m] == '-')
                    dashes++;
            }
        }
        if (dashes > max){
            max = dashes;
            mostdash = fgroups[i];
        }
    }

    printf("Group ");
    for (int k = 0; k < mostdash.size(); ++k){
        printf("%s ", mostdash[k]->bits);
    }
    printf(" has the fewest literals.\n");

    // add in the essential prime implicants
    for (int i = 0; i < implicants.size(); ++i){
        if (implicants[i]->essential)
            mostdash.push_back(implicants[i]);
    }

    sort(mostdash.begin(), mostdash.end());
    return mostdash;

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

    std::vector<Term*> min = findMin(pichart, ones, merged);

    printPIchart(pichart, ones, merged);

    printf("\n\nF = ");
    for (int i = 0; i < min.size(); ++i){
        for (int k = 0; k < min[i]->len; ++k){
            switch(min[i]->bits[k]){
                case '1':
                    printf("%c", 'A'+k);
                    break;

                case '0':
                    printf("%c'", 'A'+k);
                    break;
            }
        }
        if (i < min.size()-1)
            printf(" + ");
    }
    printf("\n");




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
