#ifndef __GI_Elements_H__
#define __GI_Elements_H__


#ifdef __cplusplus
extern "C" {
#endif


long getelements_(FileInfoPtr thisFile, long *theElements);
long setelements_(FileInfoPtr thisFile, long sizeOfElements, 
		  long numberOfElements, long *theElements);
long getelementinfo_(FileInfoPtr thisFile, long *numberOfElements,
		  long *elementSize, long *numberOfScalarValues, 
		  long *numberOfVectorValues, long *numberOfTensorValues);
long setelementscalars_(FileInfoPtr thisFile, long theType,
		     long numberOfScalars, ScalarPtr theScalarData);
long getelementscalars_(FileInfoPtr thisFile, long scalarIndex, 
		     long  *theType, ScalarPtr theScalarData);
long getelementscalartypes_(FileInfoPtr thisFile, long *theTypes);

long setelementvectors_(FileInfoPtr thisFile, long theType,
		     long numberOfVectors, VectorPtr theVectorData);
long getelementvectors_(FileInfoPtr thisFile, long vectorIndex, 
		     long  *theType, VectorPtr theVectorData);
long getelementvectortypes_(FileInfoPtr thisFile, long *theTypes);

long setelementtensors_(FileInfoPtr thisFile, long theType, long theDimension,
		     long numberOfTensors, TensorPtr theTensorData);
long getelementtensors_(FileInfoPtr thisFile, long tensorIndex,
			long *theDimension,
		     long  *theType, TensorPtr theTensorData);
long getelementtensortypes_(FileInfoPtr thisFile, long *theTypes,
			 long *theDimensions);

#ifdef __cplusplus
}
#endif

#endif
