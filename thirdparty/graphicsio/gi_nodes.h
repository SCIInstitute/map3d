#ifndef __GI_NODES_H__
#define __GI_NODES_H__

#ifdef __cplusplus
extern "C" {
#endif


long setnodes_(FileInfoPtr thisFile, long numberOfNodes, NodePtr theNodes);
long getnodeinfo_(FileInfoPtr thisFile, long *numberOfNodes,
		  long *numberOfScalarValues, long *numberOfVectorValues,
		  long *numberOfTensorValues);
long getnodes_(FileInfoPtr thisFile, NodePtr theNodeData);
long setnodescalars_(FileInfoPtr thisFile, long theType,
		     long numberOfScalars, ScalarPtr theScalarData);
long getnodescalars_(FileInfoPtr thisFile, long scalarIndex, 
		     long  *theType, ScalarPtr theScalarData);
long getnodescalartypes_(FileInfoPtr thisFile, long *theTypes);
long setnodevectors_(FileInfoPtr thisFile, long theType,
		     long numberOfVectors, VectorPtr theVectorData);
long getnodevectors_(FileInfoPtr thisFile, long vectorIndex, 
		     long  *theType, VectorPtr theVectorData);
long getnodevectortypes_(FileInfoPtr thisFile, long *theTypes);

long setnodetensors_(FileInfoPtr thisFile, long theType, long theDimension,
		     long numberOfTensors, TensorPtr theTensorData);
long getnodetensors_(FileInfoPtr thisFile, long tensorIndex, 
		     long *theDimension,
		     long  *theType, TensorPtr theTensorData);
long getnodetensortypes_(FileInfoPtr thisFile, long *theTypes,
			 long *theDimensions);

#ifdef __cplusplus
}
#endif

#endif

