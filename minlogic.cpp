// Quine-McCluskey minimization
//
// Patrick Mealey
// Joseph Gebhard

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
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
    std::vector<char> outputs;

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

bool vectorSizeCompare( const std::vector<Term*> left, const std::vector<Term*> right ){
    return left.size() < right.size();
}



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

// this could be made more efficient... 
// we shouldn't need to compare every term to every other term.
// mergeTermsOnce returns a new vector of merged terms, 
// and modifies the terms in the input vector to mark those that are essential
std::vector<Term*> mergeTermsOnce(std::vector<Term*> terms, std::vector<Term*> complist){
    std::vector<Term*> newterms;

    // mark all terms essential
    for(int i = 0; i < terms.size(); ++i){
        if (terms[i]->dontcare == false)
            terms[i]->essential = true;
    }
    for(int i = 0; i < complist.size(); ++i){
        if (complist[i]->dontcare == false)
            complist[i]->essential = true;
    }
    // for each term
    for (int i = 0; i < terms.size(); ++i){
        // for each term after i
        for (int k = 0; k < complist.size(); ++k){
            int bitdiff = -1;
            // for each bit
            for (int m = 0; m < terms[i]->len && m < complist[k]->len; ++m){
                // if bits differ
                if (terms[i]->bits[m] != complist[k]->bits[m]){
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
                complist[k]->essential = false;
                Term* new1 = new Term(*terms[i]);
                if (terms[i]->dontcare == false || complist[k]->dontcare == false)
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
                //printf("deleting %d:%s duplicate of %d:%s\n", k, newterms[k]->bits, i, newterms[i]->bits);
                delete newterms[k];
                newterms.erase(newterms.begin() + k);
                k--;
            }
        }
    }
    return newterms;
}

std::vector<Term*> mergeTerms(std::vector<Term*> terms){
    if (terms.size() == 0){
        std::vector<Term*> ret;
        return ret;
    }
    std::vector<Term*> merged;
    std::vector<Term*> lastmerged;

    std::vector<Term*> essential;

    // copy terms into merged
    std::vector<Term*>::iterator it;
    for (it = terms.begin(); it != terms.end(); ++it){
        Term* copy = new Term(*(*it));
        merged.push_back(copy);
    }

    // flag to indicate we're finished merging
    bool done = false;

    // loop until nothing can be merged.
    int count = 0;
    while(done == false){
        //printf("Looped on merge %d times\n", ++count);
        
        for (it = lastmerged.begin(); it != lastmerged.end(); ++it){
            delete (*it);	
        }

        lastmerged = merged;
        merged = mergeTermsOnce(lastmerged, lastmerged);

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
    if (terms.size() > 25){
        return;
    }
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
        int mod = (implicants[i]->len % 2 == 1 ? 0 : 1);
        printf("%s%c ", implicants[i]->bits, implicants[i]->essential ? '*' : ' ');
        for(int k = 0; k < terms.size(); ++k){
            snprintf(fmt, 50, " %%%ds%%s%%%ds ", (terms[k]->len)/2-mod, (terms[k]->len)/2);
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

        //printPIchart(table_, terms_, imps_);

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

        //printPIchart(table_, terms_, imps_);
    //}
    
    std::vector<Term*> toRemove;
    /*
    // "Row" dominance -- column dominance here
    // If a term dominates another term, then the dominating one can be ignored
    for (int i = 0; i < terms_.size(); ++i){
        for (int k = i+1; k < terms_.size(); ++k){
            bool domk = true;
            bool domi = true;
            for (int m = 0; m < imps_.size(); ++m){
                if (table_[m][i] == true && table_[m][k] == false){
                    domi = false;
                }
                if (table_[m][k] == true && table_[m][i] == false){
                    domk = false;
                }
            }
            if (domk){
                toRemove.push_back(terms_[i]);
                printf("Col %d dominates col %d\n", i, k);
            } else if (domi){
                toRemove.push_back(terms_[k]);
                printf("Col %d dominates col %d\n", k, i);
            }
        }
    } 

    // remove columns that dominate others
    printf("imps size: %d, terms size: %d\n", imps_.size(), terms_.size());
    for (int i = 0; i < terms_.size(); ++i){
        for (int k = 0; k < toRemove.size(); ++k){
            if (*(toRemove[k]) == *(terms_[i])){
                toRemove.erase(toRemove.begin() + k);
                terms_.erase(terms_.begin() + i);
                i--;
                break;
            }
        }
    }
    printf("imps size: %d, terms size: %d\n", imps_.size(), terms_.size());

    // clean up old table_
    for (int i = 0; i < imps_.size(); ++i){
        delete[] table_[i];
    }
    delete[] table_;

    table_ = buildPI(terms_, imps_);

    printPIchart(table_, terms_, imps_);
    */

    // "Column" dominance -- actually rows in the table here...
    // If a prime implicant covers another completely, then the covered one can be ignored
    toRemove.clear();
    for (int i = 0; i < imps_.size(); ++i){
        for (int k = i+1; k < imps_.size(); ++k){
            bool dom1 = true;
            bool dom2 = true;
            for (int m = 0; m < terms_.size(); ++m){
                if (table_[i][m] == true && table_[k][m] == false){
                    dom1 = false;
                }
                if (table_[k][m] == true && table_[i][m] == false){
                    dom2 = false;
                }
            }
            if (dom2){
                toRemove.push_back(imps_[k]);
                //printf("Row %d dominates row %d\n", i, k);
            } else if (dom1){
                toRemove.push_back(imps_[i]);
                //printf("Row %d dominates row %d\n", k, i);
                break;
            }
        }
    }

    // remove rows that are dominated
    //printf("imps size: %d, terms size: %d\n", imps_.size(), terms_.size());
    bool* impignore = new bool[imps_.size()];
    for (int i = 0; i < imps_.size(); ++i){
        impignore[i] = false;
    }
    int impidx = 0;
    int impsize = imps_.size();
    for (int i = 0; i < imps_.size(); ++i){
        for (int k = 0; k < toRemove.size(); ++k){
            if (*(toRemove[k]) == *(imps_[i])){
                toRemove.erase(toRemove.begin() + k);
                imps_.erase(imps_.begin() + i);
                impignore[impidx] = true;
                i--;
                break;
            }
        }
        impidx++;
    }

    // clean up the impignore table
    delete[] impignore;
    impignore = NULL;

    //printf("imps size: %d, terms size: %d\n", imps_.size(), terms_.size());

    // clean up old table_
    for (int i = 0; i < impsize; ++i){
        delete[] table_[i];
    }
    delete[] table_;

    table_ = buildPI(terms_, imps_);
    impsize = imps_.size();

    //printPIchart(table_, terms_, imps_);

    
    // Greedy algorithm
    // Always choose the implicant that covers the most uncovered terms

    // so we don't have to keep recreating the table...
    bool* covered = new bool[terms_.size()];
    for (int i = 0; i < terms_.size(); ++i){
        covered[i] = false;
    }
    bool* used = new bool[imps_.size()];
    for (int i = 0; i < imps_.size(); ++i){
        used[i] = false;
    }

    int colsleft = terms_.size();
    std::vector<Term*> chosen;
    
    // while there are uncovered cols
    while(colsleft > 0){

        // loop over implicants and find the one that covers the most terms
        int maxcols = 0;
        int impidx = 0;
        std::vector<int> atmax;
        for (int i = 0; i < imps_.size(); ++i){
            if (used[i]){
                continue;
            }
            int cols = 0;
            for (int k = 0; k < terms_.size(); ++k){
                if (covered[k]){
                    continue;
                }

                if (table_[i][k]){
                    cols++;
                }
            }
            if (cols > maxcols){
                maxcols = cols;
                impidx = i;
                atmax.clear();
                atmax.push_back(i);
            } else if (cols == maxcols){
                atmax.push_back(i);
            }
        }
        int maxdash = 0;
        int mdashidx = 0;
        // find the one with the most dashes
        for (int i = 0; i < atmax.size(); ++i){
            int dashes = 0;
            for (int m = 0; m < imps_[atmax[i]]->len; ++m){
                if (imps_[atmax[i]]->bits[m] == '-'){
                    dashes++;
                }
            }
            if (dashes > maxdash){
                maxdash = dashes;
                mdashidx = i;
            }
        }
        chosen.push_back(imps_[atmax[mdashidx]]);
        used[atmax[mdashidx]] = true;
        for (int i = 0; i < terms_.size(); ++i){
            if (table_[atmax[mdashidx]][i] && covered[i] == false){
                covered[i] = true;
                colsleft--;
            }
        }
    }
    delete[] covered;
    covered = NULL;
    delete[] used;
    used = NULL;



    // add in the essential prime implicants
    for (int i = 0; i < implicants.size(); ++i){
        if (implicants[i]->essential)
            chosen.push_back(implicants[i]);
    }

    sort(chosen.begin(), chosen.end());
    printTerms(chosen);
    return chosen;
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
    int numOuts = 1;
    int numTerms = 0;

    char firstline[100];
    infile.getline(firstline, 100);
    std::stringstream flss (firstline);

    flss >> numVars >> numOuts;

    infile >> numTerms;

    //printf("Got numVars: %d, numOuts: %d, numTerms:%d\n", numVars, numOuts, numTerms);

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
        for (int z = 0; z < numOuts; ++z){
            infile >> bit;
            term->outputs.push_back(bit);
        }

        terms.push_back(term);
    }

    // for each output...
    for (int z = 0; z < numOuts; ++z){

        // prepare currterms list
        std::vector<Term*> currterms;
        for (int i = 0; i < terms.size(); ++i){
            if (terms[i]->outputs.size() <= z){
                printf("No output%d value for term %s -- skipping\n", z, terms[i]->bits);
                continue;
            }
            if (terms[i]->outputs[z] == '0'){
                continue;
            } else if (terms[i]->outputs[z] == 'd'){
                terms[i]->dontcare = true;
            } else {
                terms[i]->dontcare = false;
            }
            currterms.push_back(terms[i]);
        }

        // merge terms
        std::vector<Term*> merged;
        merged = mergeTerms(currterms);
        
        //printf("Original Terms:\n");
        //printTerms(terms);

        //printf("Merged Terms:\n");
        printTerms(merged);


        // clear essential flags
        for (int i = 0; i < currterms.size(); ++i){
            terms[i]->essential = false;
        }
        for (int i = 0; i < merged.size(); ++i){
            merged[i]->essential = false;
        }
        
        // get the terms = 1
        std::vector<Term*> ones;
        for (int i = 0; i < currterms.size(); ++i){
            if (currterms[i]->dontcare == false)
                ones.push_back(currterms[i]);
        }
                
        // build prime implicant chart
        bool** pichart = buildPI(ones, merged);

        //printPIchart(pichart, ones, merged);

        std::vector<Term*> min = findMin(pichart, ones, merged);

        printf("\n\nOut%d = ", z);
        int lits = 0;
        for (int i = 0; i < min.size(); ++i){
            for (int k = 0; k < min[i]->len; ++k){
                switch(min[i]->bits[k]){
                    case '1':
                        printf("%c ", 'A'+k);
                        lits++;
                        break;

                    case '0':
                        printf("%c' ", 'A'+k);
                        lits++;
                        break;
                }
            }
            if (i < min.size()-1)
                printf(" + ");
        }
        printf("\nPrime Implicant Count: %d\nLiteral Count: %d\n", min.size(), lits);




        // clean up
        for (int i = 0; i < merged.size(); ++i){
            delete merged[i];
        }

        for (int i = 0; i < merged.size(); ++i){
            delete[] pichart[i];
        }
        delete[] pichart;
    }

    // clean up
    for (int i = 0; i < terms.size(); ++i){
        delete terms[i];
    }
}
