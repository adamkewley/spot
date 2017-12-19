#include "optimizer_test.h"

#include <thread>
#include <chrono>
#include "test_functions.h"

namespace spot
{
	void optimizer_test()
	{
		// setup mean / std / N
		int dim = 10;
		int seed = 123;
		int lambda = 0;
		vec_< double > init_mean( dim, 0.0 );
		vec_< double > init_std( dim, 0.3 );
		vec_< double > lower( dim, -1e18 );
		vec_< double > upper( dim, -1e18 );

		// init c-cmaes
		cmaes_t evo;
		double *arFunvals = cmaes_init( &evo, dim, &init_mean[ 0 ], &init_std[ 0 ], seed, lambda, "no" );

		// init cma_optimizer
		function_objective obj( dim, cigtab, init_mean, init_std, lower, upper );
		cma_optimizer cma( obj, lambda, seed );
		cma.set_max_threads( 10 );

		/* Iterate until stop criterion holds */
		for ( int gen = 0; gen < 100; ++gen )
		{
			// update C-CMAES
			{
				/* generate lambda new search points, sample population */
				double* const* pop;
				//printf( "C%03d: ", gen );
				pop = cmaes_SamplePopulation( &evo ); /* do not change content of pop */

				/* evaluate the new search points using fitfun */
				for ( int i = 0; i < cmaes_Get( &evo, "lambda" ); ++i ) {
					arFunvals[ i ] = cigtab_c( pop[ i ], (int)cmaes_Get( &evo, "dim" ) );
					//printf( "%.2f ", arFunvals[ i ] );
				}

				/* update the search distribution used for cmaes_SamplePopulation() */
				cmaes_UpdateDistribution( &evo, arFunvals );
				//printf( "\n" );
			}

			// update cma_optimizer
			{
				//printf( "D%03d: ", gen );
				auto& cma_pop = cma.sample_population();
				auto results = cma.evaluate( cma_pop );
				for ( int i = 0; i < cma.lambda(); ++i ) {
					//printf( "%.2f ", results[ i ] );
				}

				/* update the search distribution used for cmaes_SamplePopulation() */
				cma.update_distribution( results );
				//printf( "\n" );
			}
		}

		/* get best estimator for the optimum, xmean */
		double* xfinal = cmaes_GetNew( &evo, "xmean" ); /* "xbestever" might be used as well */
		cmaes_exit( &evo ); /* release memory */

		/* do something with final solution and finally release memory */
		free( xfinal );
	}

	void optimizer_thread_test()
	{
		// setup mean / std / N
		size_t dim = 10;
		int seed = 123;
		int lambda = 0;

		// init cma_optimizer
		function_objective obj( dim, cigtab, 0.0, 0.3, -1e18, 1e18 );
		cma_optimizer cma( obj, lambda, seed );
		cma.set_max_threads( 3 );

		for ( int gen = 0; gen < 10; ++gen )
		{
			// update cma_optimizer
			{
				auto& cma_pop = cma.sample_population();
				auto results = cma.evaluate( cma_pop );

				printf( "D%03d: ", gen );
				for ( int i = 0; i < cma.lambda(); ++i ) {
					printf( "%.2f ", results[ i ] );
				}
				printf( "\n" );

				/* update the search distribution used for cmaes_SamplePopulation() */
				cma.update_distribution( results );
			}
		}

	}

}
