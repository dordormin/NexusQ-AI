import pypdf
import sys

def extract_text(pdf_path):
    try:
        reader = pypdf.PdfReader(pdf_path)
        print(f"--- START OF {pdf_path} ---")
        for page in reader.pages:
            print(page.extract_text())
        print(f"--- END OF {pdf_path} ---")
    except Exception as e:
        print(f"Error reading {pdf_path}: {e}")

if __name__ == "__main__":
    files = [
        "docs/Architecture Système NexusQ-AI.pdf",
        "docs/Projet Tomographie.pdf",
        "docs/My Quantum Simulator.pdf",
        "docs/Créer un OS _ Synthèse de livres.pdf"
    ]
    for f in files:
        extract_text(f)
