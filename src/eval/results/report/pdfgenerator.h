#ifndef EVALUATIONRESULTSREPORTPDFGENERATOR_H
#define EVALUATIONRESULTSREPORTPDFGENERATOR_H

#include "configurable.h"

class EvaluationManager;

namespace EvaluationResultsReport
{

    class PDFGeneratorDialog;

    class PDFGenerator : public Configurable
    {
    public:
        PDFGenerator(const std::string& class_id, const std::string& instance_id,
                     EvaluationManager& eval_manager);

        virtual void generateSubConfigurable(const std::string& class_id,
                                             const std::string& instance_id);

        PDFGeneratorDialog& dialog();

        void run ();
        void cancel ();

        std::string reportPath() const;
        void reportPath(const std::string& path);

        std::string reportFilename() const;
        void reportFilename(const std::string& filename);

        void reportPathAndFilename(const std::string& str);

        bool isRunning() const;

        void showDone(bool show_done);

        std::string author() const;
        void author(const std::string& author);

        std::string abstract() const;
        void abstract(const std::string& abstract);

        bool runPDFLatex() const;
        void runPDFLatex(bool value);

        bool pdfLatexFound() const;

        bool openCreatedPDF() const;
        void openCreatedPDF(bool value);

        bool waitOnMapLoading() const;
        void waitOnMapLoading(bool value);

    protected:
        EvaluationManager& eval_man_;

        std::unique_ptr<PDFGeneratorDialog> dialog_;

        std::string report_path_;
        std::string report_filename_; // without path

        std::string author_;
        std::string abstract_;

        bool wait_on_map_loading_ {true};

        bool run_pdflatex_ {true};
        bool pdflatex_found_ {false};

        bool open_created_pdf_ {false};

        bool running_ {false};
        bool cancel_ {false};
        bool show_done_ {true};

        bool pdf_created_ {false};

        virtual void checkSubConfigurables();
    };

}

#endif // EVALUATIONRESULTSREPORTPDFGENERATOR_H
