#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <stdint.h>
#include <limits.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdint>


using std::vector;
using std::string;
using std::cout;
using std::endl;
using namespace std::chrono;

string hashFunction(string s);

class user{
private:
    string name;
    string public_key;
    int balance;

public:
    user(string defn = "", string defk = "", int defb = 0): name{defn}, public_key{defk}, balance{defb}{};
    void setName(string n) { name = n;}
	void setKey(string k) { public_key = k;}
	void setBalance(int b) { balance = b;}
    inline string getName() const { return name;}
    inline string getKey() const { return public_key;}
    inline int getBalance() const { return balance;}
    void addBalance(int toAdd) {balance+=toAdd;}
    void subBalance(int toSub) {balance-=toSub;}
};

vector<user> users;

class transaction{
public:
    string id;
    string fromHash, toHash;
    double amount;
    transaction(string defI = "", string defF = "", string defT = "", double defA = 0): id(defI), fromHash(defF), toHash(defT), amount(defA){};
};

class block{
private:
    time_t timestamp;
    float version = 1.0;
    string thisHash;
    vector<transaction> thisData;
    int64_t nonce;
    uint32_t thisIndex;
    string merkleTree;
    string calculateHash();
    string merkleRoot(vector<string> hashList);
public:
    block(uint32_t index, const vector<transaction> &data) : thisIndex(index), thisData(data) {
    nonce = -1;
    timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    };
    string previousHash;
    string getMerkle() const { return merkleTree;}
    string getHash() const { return thisHash;}
    vector <transaction> getData() const { return thisData;}
    void removeTransaction(int i){ thisData.erase(thisData.begin()+i);}
    void mineBlock(uint32_t difficulty);
};

string block::merkleRoot(vector<string> hashList){
    while(hashList.size() > 1){
    vector<string> newList;

    for(int i = 1; i < hashList.size(); i=i+2){
        newList.push_back(hashFunction(hashList[i] + hashList[i-1]));
    }
    if(hashList.size()%2 > 0) newList.push_back(hashFunction(hashList[hashList.size()-1] + hashList[hashList.size()-1]));

    hashList = newList;
    newList.clear();
    }
    merkleTree = hashList[0];
}

string block::calculateHash(){
    string toHash = "00";
    toHash += std::to_string(nonce);
    toHash += std::to_string(thisIndex);
    toHash += std::to_string(timestamp);
    toHash += previousHash;
    toHash += merkleTree;
    return hashFunction(toHash);
}

void block::mineBlock(uint32_t difficulty) {
    vector<string> hashList;
    for(int i = 0; i < thisData.size(); i++)
    {
        string temp;
        temp += thisData[i].fromHash;
        temp += thisData[i].toHash;
        temp += std::to_string(thisData[i].amount);
        hashList.push_back(temp);
    }
    merkleRoot(hashList);
    cout << getMerkle() << endl;

    char diffLenght[difficulty + 1];

    for (uint32_t i = 0; i < difficulty; ++i) {
        diffLenght[i] = '0';
    }
    diffLenght[difficulty] = '\0';

    string diffString(diffLenght);
    do{
        nonce++;
        thisHash = calculateHash();
    } while (thisHash.substr(0, difficulty) != diffString);

    cout << "Block mined: " << thisHash << endl;
}

class chain{
private:
    uint32_t difficulty;
    vector<block> thisChain;
    block getLast() { return thisChain.back();}
public:
    chain(){
    vector <transaction> temp;
    transaction t = transaction("", "Root", "Root", 0);
    temp.push_back(t);
    thisChain.push_back(block(0, temp));
    difficulty = 1;
    }
    void addBlock(block newBlock);
};

void chain::addBlock(block newBlock) {
    for(int i = 0; i < newBlock.getData().size(); i++){
        if(hashFunction(newBlock.getData()[i].toHash + newBlock.getData()[i].fromHash + std::to_string(newBlock.getData()[i].amount)) != newBlock.getData()[i].id){
            newBlock.removeTransaction(i);
            cout << "Removed transaction due to wrong transaction id" << endl;
        }
        else{
            int k = 0;
                while(newBlock.getData()[i].fromHash != users[k].getKey()){
                    k++;
                }
            k--;
        if(newBlock.getData()[i].amount > users[k].getBalance()){
            cout << "Removed transaction due to insufficient sender funds, amount: " << newBlock.getData()[i].amount << endl;
            newBlock.removeTransaction(i);
            }
        }
    }
    newBlock.previousHash = getLast().getHash();
    newBlock.mineBlock(difficulty);
    thisChain.push_back(newBlock);
}

uint64_t** convertDecimal(string s, int & n, int & sizeOfMessage){
    sizeOfMessage = s.size();

	n = sizeOfMessage/32;
	if(sizeOfMessage%32 == 0) n--;

	for(int i = 0; i < (n+1)*32-sizeOfMessage; i++) s+="0";

	uint64_t** decimalArray = 0;
	decimalArray = new uint64_t*[n+1];
	int pos = 0;

	for (int h = 0; h < n+1; h++)
      {
            decimalArray[h] = new uint64_t[4];

            for (int w = 0; w < 4; w++)
            {
                string temp = s.substr(pos, 8);

                uint64_t ans = (uint64_t)temp[0];
                for(int i = 1; i < 8; i++){
                    ans = ans | (uint64_t)(temp[i] << 8*i);
                }
                decimalArray[h][w] = ans;
                pos+=8;
            }
      }
	return decimalArray;
}

uint64_t rotl (uint64_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
  c &= mask;
  return (n<<c) | (n>>( (-c)&mask ));
}

uint64_t rotr (uint64_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
  c &= mask;
  return (n>>c) | (n<<( (-c)&mask ));
}

uint64_t hashCycle(uint64_t ** decimalArray, int k, int sizeOfMessage){
    uint64_t result = 2410744073255765114781;
    for(int i = 0; i < 4; i++){
        decimalArray[k][i] = decimalArray[k][i] ^ (result*sizeOfMessage/(i+1));
    }
    for(int l = 0; l < 4; l++){
            result = result + rotl(decimalArray[k][l], 7);
            rotr(result, 7);
    for(int j = 0; j < 4; j++){
            result = result ^ rotl(decimalArray[k][j], 5);
            rotr(result, j*3);
    for(int i = 0; i < 4; i++){
            result = result * rotl(decimalArray[k][i], 3) ^ rotr(result, 9);
            rotr(result, i+1);
    }
    }
    }
    return result;
}

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

string hashFunction(string s){
    int n = 0;
    int sizeOfMessage = 0;

    uint64_t** myArray = convertDecimal(s, n, sizeOfMessage);

    uint64_t result = hashCycle(myArray, 0, sizeOfMessage);
    for(int i = 1; i < n; i++){
        result = hashCycle(myArray, i, sizeOfMessage) ^ result;
    }
    return n2hexstr(result);
}

void createUsers(vector<user>& users, int n){
    for(int i = 0; i < n; i++){
        user temp;
        temp.setName("User"+std::to_string(i+1));
        temp.setKey(hashFunction(std::to_string(i)));
        temp.setBalance(rand()%99900 + 100);
        users.push_back(temp);
    }
}

void createTransaction(vector<transaction> &transactions, int n, vector<user>& users){
    for(int i = 0; i < n; i++){
        transaction temp = transaction("", users[rand()%users.size()].getKey(), users[rand()%users.size()].getKey(), rand()%1500+1);
        temp.id = hashFunction(temp.toHash + temp.fromHash + std::to_string(temp.amount));
        transactions.push_back(temp);
    }
    transactions[72].id = "fijgfdijf";
}

int main()
{
    srand (time(NULL));
    createUsers(users, 1000);

    vector<transaction> transactions;
    createTransaction(transactions, 1000, users);

    chain bChain = chain();
    int i = 0;
    while(transactions.size() > 0){
        vector<transaction> data;
        for(int j = 0; j < 100 & transactions.size() > 0; j++){
        int selector = rand()%transactions.size();
        data.push_back(transactions[selector]);
        transactions.erase (transactions.begin()+selector);
        }
        cout << "Mining block " << i+1 <<"..." << endl;
        bChain.addBlock(block(i+1, data));
        i++;
    }
}
