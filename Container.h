///============================================================================
///======	class CContainer
///============================================================================

template<typename T>
class CContainer
{
public:
	CContainer(char * pBuffer, unsigned nBufferSize);	///======	Constructs the container from a pre-defined buffer.
	~CContainer();

	T *				Add();				///======	Adds an element to the container, constructs it and returns it to the caller.
										///======	Note that the return address needs to be persistent during the lifetime of the object.

	void			Remove(T *);		///======	Removes an object from the container.

	unsigned		Count() const;		///======	Number of elements currently in the container.
	unsigned		Capacity() const;	///======	Max number of elements the container can contain. You can limit the capacity to 65536 elements if this makes your implementation easier.

	bool			IsEmpty() const;	///======	Is container empty?
	bool			IsFull() const;		///======	Is container full?

	T const *		operator [] (unsigned nIndex) const;	///======	Returns the n th element of the container [0..Count -1].
	T *				operator [] (unsigned nIndex);			///======	Returns the n th element of the container [0..Count -1].

#ifdef _ADVANCED
	void			Sort(int (* Compare)(T const * pX, T const * pY));	///======	Sort the elements using a function pointer for the comparison.
#endif
};
