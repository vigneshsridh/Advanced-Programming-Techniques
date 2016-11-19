// ECE4122/6122 RSA Encryption/Decryption assignment
// Fall Semester 2016
//VIGNESH SRIDHARAN

#include <iostream>
#include "RSA_Algorithm.h"

using namespace std;

int main()
{
  // Instantiate the one and only RSA_Algorithm object
  RSA_Algorithm RSA;

   
  // For each key pair choose 100 differnt plaintext 
  // messages making sure it is smaller than n.
  // If not smaller then n then choose another
  // For eacm message encrypt it using the public key (n,e).
  // After encryption, decrypt the ciphertext using the private
  // key (n,d) and verify it matches the original message.

  // your code here
  mpz_class sz, p , q, n, phi, d, e;
  
  bool fin1, fin2;

  // Loop from sz = 32 to 1024 inclusive
  for(sz = 32; sz <= 1024; sz = sz = sz * 2) {
  	// for each size choose 100 different key pairs
	for(int i = 0; i < 100; i++) {
    	fin1 = false; fin2 = false;
    	// Step 1: Find random prime number p of size sz bits
    	while(!fin1) {	
      		p = RSA.rng.get_z_bits(sz);
      		int num = mpz_probab_prime_p(p.get_mpz_t(), 100);
      		if(num == 2 || num == 1)	//0 indicates NOT PRIME
        		fin1 = true;
    	}

    	//Step 2: Find random prime number q of size sz bits
    	while(!fin2) {	
      		q = RSA.rng.get_z_bits(sz);
      		int num = mpz_probab_prime_p(q.get_mpz_t(), 100);
      		if(num == 2 || num == 1)	//0 indicates NOT PRIME
        		fin2 = true;
    	}
    	
    	// Step 3: Compute n = p * q
    	n = p * q;

    	// Step 4: Compute phi = (p-1) * (q-1)
    	phi = (p - 1) * (q - 1);

    	// Step 5: Choose d of size sz * 2 such that 1 < d < phi and gcd(d,phi) = 1
    	bool fin3 = false;
    	while(!fin3) {
      		d = RSA.rng.get_z_bits(sz * 2);
      		mpz_class gcd;
    		mpz_gcd(gcd.get_mpz_t(), d.get_mpz_t(), phi.get_mpz_t());
    		if(d < phi && gcd == 1)
      			fin3 = true;
    	}

    	//Step 6: Compute e as the multiplicative inverse of d mod(phi)
        //mpz_t e_int; 
        int inv_exist = mpz_invert(e.get_mpz_t(), d.get_mpz_t(), phi.get_mpz_t());
    	if(!inv_exist) 
      		cout<<"Inverse doesn't exist for this combination\n";

      	//Now (n,e) is the public key and (n,d) is the private key

    	RSA.n = n;
    	RSA.d = d;
    	RSA.e = e;
    	RSA.PrintNDE();

    	mpz_class msg, c, m_num, d;
    	for(int messagenum = 0; messagenum < 100; messagenum++) {
      
      		msg = RSA.rng.get_z_bits(2 * sz - 1);
		while(msg > RSA.n) {
			msg = RSA.rng.get_z_bits(2 * sz - 1);
		}

		RSA.PrintM(msg);
      		//cout<<"Message: "<<msg<<endl;
      		// mpz_powm generates: output = (input ^ key)mod(n)
      		// Encrypt() and Decrypt() methods hence use mpz_powm
      		
      		mpz_class ciphertext = RSA.Encrypt(msg);
		RSA.PrintC(ciphertext);
      		
      		mpz_class decrypted = RSA.Decrypt(ciphertext);
		//RSA.PrintM(d);
		/*if(decrypted != msg) {
			cout<<"Does not match!"<<endl;
			RSA.PrintM(msg);
			RSA.PrintC(ciphertext);
			cout<<"D "<<decrypted<<endl;
			RSA.PrintNDE();
			return 0;			
		}*/
    	}
  	}
 }

}
  
