#!/bin/bash

usage() {
    echo "Usage: $0 [option] -i <IP>"
    echo "Options:"
    echo "  -o       : Affiche 'Option -o choisie' et exécute ./test ip 99090 3233"
    echo "  -c       : Affiche 'Option -c choisie' et exécute ./test ip 9872 782"
    echo "  -ro      : Affiche 'Option -ro choisie'"
    echo "  -rc      : Affiche 'Option -rc choisie'"
    echo "  -i <IP>  : Spécifie l'adresse IP à utiliser (obligatoire)"
    exit 1
}


if [ $# -eq 0 ]; then
    usage
fi

OPTION=""
IP=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -i)
            if [[ -n "$2" && "$2" != -* ]]; then
                IP="$2"
                shift
            else
                echo "Erreur : L'option -i nécessite une adresse IP."
                usage
            fi
            ;;
        -o)
            OPTION="o"
            ;;
        -c)
            OPTION="c"
            ;;
        -ro)
            OPTION="ro"
            ;;
        -rc)
            OPTION="rc"
            ;;
        *)
            echo "Option inconnue : $1"
            usage
            ;;
    esac
    shift
done

if [ -z "$IP" ]; then
    echo "Erreur : L'option -i avec une adresse IP est obligatoire."
    usage
fi

case "$OPTION" in
    o)
        knock "$IP" 7000 8000 9000
        ;;
    c)
        knock "$IP" 9000 8000 7000
        ;;
    ro)
        knock "$IP" 7001 8001 9001
        ;;
    rc)
        knock "$IP" 9001 8001 7001
        ;;
    *)
        echo "Erreur : Aucune option valide n'a été choisie."
        usage
        ;;
esac
