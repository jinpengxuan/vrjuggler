
#include <LabeledPerfDataBufferTest.h>

#include <iostream>

//#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include <jccl/Plugins/PerformanceMonitor/LabeledPerfDataBuffer.h>
#include <jccl/Plugins/PerformanceMonitor/TimeStamp.h>
#include <jccl/Util/Debug.h>
#include <vpr/Thread/TSObjectProxy.h>
#include <vpr/Thread/Thread.h>
#include <vpr/Sync/Barrier.h>
// #include <jccl/Config/VarValue.h>
// #include <jccl/Config/ConfigChunk.h>
// #include <jccl/Config/ChunkDesc.h>

/*****************************************************************
 tests out the functionality expected of jccl::LabeledPerfDataBuffer
*******************************************************************/

namespace jcclTest
{


    LabeledPerfDataBufferTest::LabeledPerfDataBufferTest() : CppUnit::TestCase ()
    {
    }
    
    LabeledPerfDataBufferTest::LabeledPerfDataBufferTest(std::string name) : CppUnit::TestCase (name)
    {
    }

    /*virtual*/ LabeledPerfDataBufferTest::~LabeledPerfDataBufferTest()
    {
    }


    void LabeledPerfDataBufferTest::testSetOverhead() {

        int i, n;
        i = n = 100000;

        jccl::PerformanceCategories::instance()->activate();

        jccl::PerformanceCategories::instance()->activateCategory ("PERF_JACKAL");

        jccl::LabeledPerfDataBuffer buffer1("testSetOverhead buffer 1", n+1);

        jccl::TimeStamp ts1, ts2;

        ts1.set();

        while(i--) {
            buffer1.set(jcclPERF_JACKAL, "foobar");
        }

        ts2.set();

        double average = (ts2.usecs() - ts1.usecs())/n;

        std::cout << "jccl::LabeledPerfDataBuffer::set()\n";
        std::cout << "\tOverhead = " << average << " us per call" << std::endl;

        vpr::TSObjectProxy<jccl::LabeledPerfDataBuffer> tsbuffer;

        tsbuffer->setBufferSize(101000);

        i = n;

        ts1.set();

        while(i--) {
            tsbuffer->set(jcclPERF_JACKAL, "foobar");
        }

        ts2.set();

        average = (ts2.usecs() - ts1.usecs())/n;

        std::cout << "jccl::LabeledPerfDataBuffer::set() (Thread-specific)\n";
        std::cout << "\tOverhead = " << average << " us per call" << std::endl;


    }


    // test for set overhead with multithread contention
    struct SetContentionTestLoopData {
        vpr::Barrier* barrier;
        int thread_num;
        vpr::TSObjectProxy<jccl::LabeledPerfDataBuffer>* tsbuffer;
        SetContentionTestLoopData (vpr::Barrier* _barrier, 
                                   int _thread_num,
                                   vpr::TSObjectProxy<jccl::LabeledPerfDataBuffer>* _tsbuffer) {
            barrier = _barrier;
            thread_num = _thread_num;
            tsbuffer = _tsbuffer;
        }
    };

    void LabeledPerfDataBufferTest::setContentionTestLoop (void* d) {
        SetContentionTestLoopData* data = (SetContentionTestLoopData*)d;
        char names[64];
        sprintf (names, "thread %d", data->thread_num);
        std::string name (names);
        int i = 100000;
        vpr::TSObjectProxy<jccl::LabeledPerfDataBuffer>* buffer = data->tsbuffer;
        (*buffer)->setBufferSize(101000);
        vpr::GUID new_cat;
        data->barrier->wait();
        while (i--) {
            (*buffer)->set(jcclPERF_JACKAL, name);
            //jccl::PerformanceCategories::instance()->isCategoryActive (jcclPERF_JACKAL);
            //new_cat = jcclPERF_JACKAL;
        }
        data->barrier->wait();
    }
        

    void LabeledPerfDataBufferTest::testSetContentionOverhead () {
        //jccl::PerformanceCategories::instance()->activate();

        vpr::TSObjectProxy<jccl::LabeledPerfDataBuffer> tsbuffer;
        jccl::TimeStamp ts1, ts2;

        int num_threads = 6;
        int n = 100000;
        vpr::Barrier b(num_threads+1);

        for (int i = 0; i < num_threads; i++) {
            
            vpr::ThreadMemberFunctor<LabeledPerfDataBufferTest>* memberFunctor =
            new vpr::ThreadMemberFunctor<LabeledPerfDataBufferTest>
                (this,
                 &LabeledPerfDataBufferTest::setContentionTestLoop,
                 new SetContentionTestLoopData (&b, i, &tsbuffer));
            new vpr::Thread(memberFunctor);
        }

        ts1.set();
        // trigger;
        b.wait();
        // wait for finish;
        b.wait();
        ts2.set();


        float average = (ts2.usecs() - ts1.usecs())/n;

        std::cout << "jccl::LabeledPerfDataBuffer::set() (6 thread contention)\n";
        std::cout << "\tOverhead = " << average << " us per call" << std::endl;

    }

        

    /*static*/ CppUnit::Test* LabeledPerfDataBufferTest::suite()
    {
        CppUnit::TestSuite* test_suite = new CppUnit::TestSuite("LabeledPerfDataBufferTest");
        return test_suite;
    }

    /*static*/ CppUnit::Test* LabeledPerfDataBufferTest::metric_suite() {
        CppUnit::TestSuite* test_suite = new CppUnit::TestSuite("LabeledPerfDataBufferTest_metric");

        test_suite->addTest( new CppUnit::TestCaller<LabeledPerfDataBufferTest>("testSetOverhead", &LabeledPerfDataBufferTest::testSetOverhead));

        test_suite->addTest( new CppUnit::TestCaller<LabeledPerfDataBufferTest>("testSetContentionOverhead", &LabeledPerfDataBufferTest::testSetContentionOverhead));
        
        return test_suite;
   }



}; // namespace jcclTest

