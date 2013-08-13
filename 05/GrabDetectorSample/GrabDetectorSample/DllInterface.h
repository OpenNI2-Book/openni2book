/**
 * DLL Interface classes for C++ Binary Compatibility article
 * article at http://aegisknight.org/cppinterface.html
 *
 * Adapted from code by:     Ben Scott   (bscott@iastate.edu)
 * article author:			 Chad Austin (aegis@aegisknight.org)
 */


#ifndef DLL_INTERFACE_H
#define DLL_INTERFACE_H


// DLLs in Windows should use the standard calling convention
#ifndef DLL_CALL
#  if defined(WIN32) || defined(_WIN32)
#    define DLL_CALL __stdcall
#  else
#    define DLL_CALL
#  endif
#endif

// Export functions from the DLL
#ifndef DLL_DECL
#  if defined(WIN32) || defined(_WIN32)
#    ifdef LABS_EXPORTS
#      define DLL_DECL __declspec(dllexport)
#    else
#      define DLL_DECL
#    endif
#  else
#    define DLL_DECL
#  endif
#endif

/**
 * Interfaces exposed across a DLL boundary should derive from this
 * class to ensure that their memory is released cleanly and
 * safely. This class should be used in conjunction with the DLLImpl
 * template. <b>Your interface should NOT define a destructor!</b>
 *
 * <h3>Example Usage</h3>
 * \code
 *    class MyInterface : public DLLInterface {
 *       // MyInterface method declarations here...
 *       // should NOT include a destructor!
 *    };
 * \endcode
 *
 * @see DLLImpl */
class DLLInterface {
protected:
  /**
   * Handles the destruction of this class. This is essentially a destructor
   * that works across DLL boundaries.
   */
  virtual void DLL_CALL destroy() = 0;

public:
  /**
   * Specialized delete that calls destroy instead of the destructor.
   */
  void operator delete(void* p) {
    if (p) {
      DLLInterface* i = static_cast<DLLInterface*>(p);
      i->destroy();
    }
  }
};

// Factory function example:
//extern "C" DLL_DECL MyInterface* DLL_CALL FactoryFunction(void* args);

#endif