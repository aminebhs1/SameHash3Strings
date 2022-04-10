// SameHash3Strings.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <mutex>
#include <atomic>
#include <thread>
#include <Windows.h>
#include <limits>

#undef max

using namespace std;

class CollisionGenerator {
public:
    void generateAndPrint3StringCollisionMultithreaded()
    {
        init();
        vector<thread> threads(nbThreads);
        for (int i{ 0 }; i < nbThreads; ++i)
        {
            threads[i] = generate3StringCollisionThread();
        }
        for (int i{ 0 }; i < nbThreads; ++i)
        {
            threads[i].join();
        }
        vector<string> collisionStrings = hashToStrings[collisionHash];
        printCollisionStrings(collisionStrings);
    }
    void generateAndPrint3StringCollision()
    {
        init();
        generate3StringCollision();
        vector<string> collisionStrings = hashToStrings[collisionHash];
        printCollisionStrings(collisionStrings);
    }
private:
    void init()
    {
        collisionHash = 0;
        collisionFound = false;
    }
    thread generate3StringCollisionThread()
    {
        return thread([&] { generate3StringCollision(); });
    }
    void generate3StringCollision()
    {
        srand(static_cast<unsigned int>(time(nullptr)) + GetCurrentThreadId());
        while (!collisionFound)
        {
            string randomString = generateRandomString();
            int hashOfRandomString = get4ByteHashCodeFromString(randomString);
            bool isHashFound{ hashToStrings.find(hashOfRandomString) != hashToStrings.end() };
            vector<string>& hToStrings = hashToStrings[hashOfRandomString];
            if (isHashFound)
            {
                if (hToStrings.size() > 2)
                {
                    continue;
                }
                bool isNewString{ true };
                for (const string& st : hToStrings)
                {
                    if (st == randomString)
                    {
                        isNewString = false;
                        break;
                    }
                }
                std::lock_guard<std::mutex> lock(mtex);
                if (isNewString)
                {
                    hToStrings.push_back(randomString);
                }
                if (hToStrings.size() > 2)
                {
                    collisionHash = hashOfRandomString;
                    collisionFound = true;
                }
            }
            else
            {
                std::lock_guard<std::mutex> lock(mtex);
                hToStrings.push_back(randomString);
            }
        }
    }
    string generateRandomString()
    {
        //const int stringLen = rand() % maxStringLen + 1;
        string randomString;
        for (int i{ 0 }; i < maxStringLen; ++i)
        {
            randomString += (char)(rand() % (int)(numeric_limits<unsigned char>::max()));
        }
        return randomString;
    }
    void printCollisionStrings(const vector<string>& collisionStrings)
    {
        if (checkAllStringsDifferAndCollide(collisionStrings))
        {
            cout << "Found " << collisionStrings.size() << " strings that share the same hash:\nHash : "
                << get4ByteHashCodeFromString(collisionStrings[0]) << "\n";
            for (int i{ 0 }; i < collisionStrings.size(); ++i)
            {
                cout << "String " << i + 1 << " : \"" << collisionStrings[i] << "\"\n";
            }
        }
        else
        {
            cout << "Something went wrong, please try again!\n";
        }
    }
    int get4ByteHashCodeFromString(const string& stringToHash) const
    {
        static hash<string> stringHasher;
        int narrowHash = static_cast<int>(stringHasher(stringToHash));
        return narrowHash;
    }
    bool checkAllStringsDifferAndCollide(const vector<string>& collisionStrings)
    {
        if (collisionStrings.size() < 2)
        {
            return false;
        }
        int h = get4ByteHashCodeFromString(collisionStrings[0]);
        for (int i{ 0 }; i < collisionStrings.size(); ++i)
        {
            if (get4ByteHashCodeFromString(collisionStrings[i]) != h)
            {
                return false;
            }
            for (int j{ i + 1 }; j < collisionStrings.size(); ++j)
            {
                if (collisionStrings[i] == collisionStrings[j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    mutex mtex;
    const char maxStringLen{ 5 };
    map<int, vector<string> > hashToStrings;
    atomic<bool> collisionFound{ false };
    int collisionHash{ 0 };
    const char nbThreads{ 4 };
};

int main()
{
    CollisionGenerator collisionGenerator;
    char choice{ 'a' };
    while (choice != 'x')
    {
        switch (choice)
        {
        case 'g':
            cout << "Processing...\n";
            collisionGenerator.generateAndPrint3StringCollisionMultithreaded();
            choice = 'a';
            break;
        default:
            cout << "Enter x to exit or g to generate a triplet of colliding strings\n";
            cin >> choice;
            if (choice == 'x')
                cout << "Exiting...\n";
            break;
        }  
    }
    return 0;
}