#include <QException>

#include "CodeEditorTest.hpp"
#include "EditorWindowTest.hpp"
#include "BackendTest.hpp"
#include "RendererTest.hpp"
#include "SettingsBackendTest.hpp"
#include "RendererTest.hpp"
#include "CodeHighlighterTest.hpp"

/**
 * @brief The Tests struct
 *
 * Implements a name/factory function pair.
 */
struct Tests{
    QString name;
    std::shared_ptr<QObject> (*testclass)();
};

/**
 * @brief The factory function
 * @return a newly created reference to a QObject
 *
 * Factory function that creates a templated object.
 */
template<typename T>
auto factory() -> std::shared_ptr<QObject> {
    return std::shared_ptr<T>(new T());
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return a sum of all failed tests
 *
 * Minimal testing toolkit. All tests
 * can be executed at once by either specifiing nothin
 * or "runall". If tests are to be run individually,
 * the class name has to be specified.
 */
int main(int argc, char** argv){
    QApplication app(argc, argv);
    int status = 0, statusSum = 0;

    struct Tests testcases[] = {
            {QStringLiteral("CodeEditor"), factory<CodeEditorTest>},
            {QStringLiteral("EditorWindow"), factory<EditorWindowTest>},
            {QStringLiteral("SettingsBackend"), factory<SettingsBackendTest>},
            {QStringLiteral("Renderer"), factory<RendererTest>},
            {QStringLiteral("Backend"), factory<BackendTest>},
            {QStringLiteral("CodeHighlighter"), factory<CodeHighlighterTest>}
            };
	    
    unsigned int size = sizeof(testcases)/sizeof(Tests);
    std::unique_ptr<int[]> codes(new int[size]);

    auto testRun = [&status, &statusSum, &codes](QObject* testObject, QString name, int i) {
                        try{
                            status = QTest::qExec(testObject, 0, 0);
                        }catch(QException e){
                            qDebug() << name << "Test threw QException:" << e.what();
                        }catch(const std::exception& e){
                            qDebug() << name << "Test threw STL Exception:" << e.what();
                        }
                        if(status != 0){
                            codes[i-1] = status;
                            statusSum += status;
                            status = 0;
                        }
                        qDebug() << "";
                    };

    if(argc == 1 || QString(argv[1]) == "runall")
        for(unsigned int i = 0; i < size; i++)
            testRun((testcases[i].testclass()).get(), testcases[i].name, i);
    else
        for(int i = 1; i < argc; i++){
            for(struct Tests &t: testcases)
                if(argv[i] == t.name){
                    testRun((testcases[i].testclass()).get(), t.name, i-1);
                    break;
                }
        }

    for(unsigned int i = 0; i < size; i++)
        if(codes[i] != 0)
            qDebug() << "Testclass" << (testcases[i+1].name) << "failed" << codes[i] << "test(s).";
    qDebug() << "Ran tests for" << size << "classes.";
    qDebug() << "Total failed tests:" << statusSum;

    return statusSum;
}
