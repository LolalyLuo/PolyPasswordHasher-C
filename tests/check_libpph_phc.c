/* Check the PHS function for the password hashing competition.
 *
 * @author  Santiago Torres
 * @date    03/27/2014 
 * @license MIT
 */
#include<check.h>
#include"libgfshare.h"
#include"libpolypasshash.h"
#include<stdlib.h>
#include<strings.h>
#include<time.h>


// we will test for extraneous input on the PHS function first
START_TEST(test_PHS_extraneous_input)
{ 
  int returnval;
  uint8 output[DIGEST_LENGTH];
  uint8 salt[SALT_LENGTH];
  uint8 password[] = {"testpassword"};



  // test for null pointers
  returnval = PHS(output, DIGEST_LENGTH, password, strlen(password), NULL,
      SALT_LENGTH, 0, 0);
  ck_assert(returnval == -1);

  returnval = PHS(NULL, DIGEST_LENGTH, password, strlen(password), salt, 
      SALT_LENGTH, 0,0);
  ck_assert(returnval == -1);

  returnval = PHS(output, DIGEST_LENGTH, NULL, strlen(password), salt,
     SALT_LENGTH, 0, 0);
  ck_assert(returnval == -1);
  
  // now let's test for wrong values in the length parameters
  returnval = PHS(output, DIGEST_LENGTH + 1, password, strlen(password), salt,
     SALT_LENGTH, 0, 0);
  ck_assert(returnval == -1);

  returnval = PHS(output, 0, password, strlen(password), salt, SALT_LENGTH, 0,
     0);
  ck_assert(returnval == -1);

  returnval = PHS(output, DIGEST_LENGTH, password, 0, salt, SALT_LENGTH, 0, 0);
  ck_assert(returnval == -1);

  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH +1, salt,
     SALT_LENGTH, 0, 0);
  ck_assert(returnval == -1);

  returnval = PHS(output, DIGEST_LENGTH, password, strlen(password), salt,
    0, 0, 0);
  ck_assert(returnval == -1);

  returnval = PHS(output, DIGEST_LENGTH, password, strlen(password), salt,
     SALT_LENGTH + 1, 0, 0);
  ck_assert(returnval == -1);

  // lets do a valid generation, we should get a 0 error
  returnval = PHS(output, DIGEST_LENGTH, password, strlen(password), salt,
     SALT_LENGTH, 0, 0);
  ck_assert(returnval == 0); 
  
}
END_TEST





// We will now test the generated input with non-ascii ranges.
START_TEST(test_PHS_input_ranges)
{
  int returnval;
  uint8 output[DIGEST_LENGTH];
  uint8 salt[SALT_LENGTH];
  uint8 precomputed_hash[DIGEST_LENGTH];
  uint8 password[PASSWORD_LENGTH];
  uint8 salted_password[SALT_LENGTH + PASSWORD_LENGTH];
  unsigned int i;
  unsigned int incidences, max_incidences = 5; 

  // generate a full range password (from 0 to 255)
  for(i=0;i<PASSWORD_LENGTH;i++){
    password[i] = i%255;
  }

  // precompute the hash to produce.
  memcpy(salted_password, salt, SALT_LENGTH);
  memcpy(salted_password + SALT_LENGTH, password, PASSWORD_LENGTH);
  _calculate_digest(precomputed_hash, salted_password, 
      SALT_LENGTH + PASSWORD_LENGTH);

  // test with full range password
  incidences = 0;
  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH, salt,
      SALT_LENGTH, 0, 0);
  ck_assert(returnval == 0);

  // we will check how obscure the hash is, we want the resulting octets to
  // not resemble the original hash. 
  for(i=0;i<DIGEST_LENGTH; i++){
    if(output[i] == precomputed_hash[i]){
      incidences ++;
    } 
  }
  ck_assert(incidences < max_incidences);


  // test with a random password
  get_random_salt(PASSWORD_LENGTH, password);

  // precompute the hash to produce.
  memcpy(salted_password, salt, SALT_LENGTH);
  memcpy(salted_password + SALT_LENGTH, password, PASSWORD_LENGTH);
  _calculate_digest(precomputed_hash, salted_password, 
      SALT_LENGTH + PASSWORD_LENGTH);

  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH, salt,
      SALT_LENGTH, 0, 0);

  // check results
  incidences = 0;
  ck_assert(returnval == 0);
  for(i=0;i<DIGEST_LENGTH; i++){
    if(output[i] == precomputed_hash[i]){
      incidences++;
    }
    
  }
  ck_assert(incidences < max_incidences);
 

  // add a full_range salt.
  for( i = 0; i < SALT_LENGTH; i++){
    salt[i] = 255-i;
  }
  
  // precompute the hash to produce.
  memcpy(salted_password, salt, SALT_LENGTH);
  memcpy(salted_password + SALT_LENGTH, password, PASSWORD_LENGTH);
  _calculate_digest(precomputed_hash, salted_password, 
      SALT_LENGTH + PASSWORD_LENGTH);


  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH, salt,
      SALT_LENGTH, 0, 0);
  
  // check results
  incidences = 0;
  ck_assert(returnval == 0);
  for(i=0;i<DIGEST_LENGTH; i++){
    if(output[i] == precomputed_hash[i]){
      incidences++;
    }
    
  }
  ck_assert(incidences < max_incidences);
 
  
  // add random salt
  get_random_salt(SALT_LENGTH, salt);

  // precompute the hash to produce.
  memcpy(salted_password, salt, SALT_LENGTH);
  memcpy(salted_password + SALT_LENGTH, password, PASSWORD_LENGTH);
  _calculate_digest(precomputed_hash, salted_password, 
      SALT_LENGTH + PASSWORD_LENGTH);

  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH, salt,
     SALT_LENGTH, 0, 0);

  // check results
  incidences = 0;
  ck_assert(returnval == 0);
  for(i=0;i<DIGEST_LENGTH; i++){
    if(output[i] == precomputed_hash[i]){
      incidences++;
    }
    
  }
  ck_assert(incidences < max_incidences);
}
END_TEST






// We will try to see if there are information leakages depending on the
// input.
START_TEST(test_PHS_timing)
{
  int returnval;
  uint8 output[DIGEST_LENGTH];
  uint8 salt[SALT_LENGTH];
  uint8 password[PASSWORD_LENGTH];

  // we will use these variables to time the function.
  time_t  tsalt0,tpassword0, tpassword1, tpolyhash0, tpolyhash1; 
  clock_t csalt0,cpassword0, cpassword1, cpolyhash0, cpolyhash1;

  // initialize the values
  tsalt0 = time(NULL);
  csalt0 = clock();
  get_random_salt(SALT_LENGTH, salt);

  tpassword0 = time(NULL);
  cpassword0 = clock();
  get_random_salt(PASSWORD_LENGTH, password);

  tpolyhash0 = time(NULL);
  cpolyhash0 = clock();

  returnval = PHS(output, DIGEST_LENGTH, password, PASSWORD_LENGTH, salt,
     SALT_LENGTH, 0, 0);
  
  tpolyhash1 = time(NULL);
  cpolyhash1 = clock();

  ck_assert(returnval == 0);

  printf("\nsalt generation time = (%f,%ld)", 
      (float)(cpassword0 - csalt0)/CLOCKS_PER_SEC, (long)(tpassword0 - tsalt0));
  printf("\nrandom password time = (%f,%ld)",
      (float)(cpolyhash0 - cpassword0)/CLOCKS_PER_SEC, 
      (long)(tpolyhash0 - tpassword0));
  printf("\npolyhash creation time = (%f,%ld)",
      (float)(cpolyhash1 - cpolyhash0)/CLOCKS_PER_SEC,
      (long)(tpolyhash1 - tpolyhash0)); 
  printf("\nTotal time : (%f,%ld)\n", 
      (float)(cpolyhash1 - csalt0)/CLOCKS_PER_SEC, (long)(tpolyhash1 - tsalt0));

}
END_TEST





// suite declaration
Suite * polypasshash_PHS_suite(void)
{
  Suite *s = suite_create ("PHS");

  // Input consistency, ranges and speed
  TCase *tc_phs = tcase_create ("phs_inputs");
  tcase_add_test(tc_phs, test_PHS_extraneous_input);
  tcase_add_test(tc_phs, test_PHS_input_ranges);
  tcase_add_test(tc_phs, test_PHS_timing);
  suite_add_tcase (s, tc_phs);

  return s;
}

int main (void)
{
  int number_failed;
  Suite *s =  polypasshash_PHS_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


