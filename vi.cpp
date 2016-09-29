//d1reportAllClassLayout 

class IVector {
	int ixx;
	//virtual ~IVector() = 0;
	virtual int get(int) const = 0;
	virtual void set(int,int) = 0;
	virtual int size() const = 0;
};

class IVirtualInheritVector : public virtual IVector {
	int get(int) const override {
		return 0;
	}
	void set(int, int) override {
	}
	int size() const override {
		return 0;
	}
};

class IVirtualInheritVector2 : public virtual IVector {
};

class IVirtualInheritVector3 : public IVirtualInheritVector2, public IVirtualInheritVector {
};

class IVirtualInheritVector4 : public IVirtualInheritVector2, public IVirtualInheritVector3 {
};

void main()
{
}