#include <iostream>

using namespace std;

int main() {

    int n = 0;
    float mean = 0;

    printf("How many: ");
    cin >> n;

    //Create an array
    int* arr = new int[n];

    for(int i = 0; i < n; i++){
        cout << i << ": ";
        cin >> arr[i];
    }

    for(int i = 0; i < n; i++){
        mean = mean + arr[i];
    }

    mean = mean / n;

    cout << "Mean: " << mean << endl;


}