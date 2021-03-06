#!/usr/bin/env bash

argv0=$0

names="
DirectionParamsValue
OrgDirection
TargetOrganization
Campaign
TermsAdmission
CampaignAchievement
AdmissionVolume
DistributedAdmissionVolume
CompetitiveGroup
EntranceTest
EntranceTestLocation
ServiceEntrant
Identification
Document
TargetContract
ServiceEntrantPhotoFile
OriginalEducationDocument
ServiceApplication
ServiceApplicationAchievement
ServiceApplicationBenefit
EntranceTestAgreedList
EntranceTestResultList
EntranceTestSheet
SendRefreshedEntranceTestDataToEpguList
CompetitiveGroupApplicationsList
OrderAdmissionPackage
EditApplicationStatusList
ServiceApplicationNoticeList
"

usage() {
	echo "usage: $argv0 <routine> <in> <out>" 1>&2 
	exit 1
}

if test -z $1 ; then
    usage
fi

if test $1 = -r ; then
	shift
	names=$( (for i in $names ; do echo $i; done) | tac )
fi

if test -z $1 || test -z $2 || test -z $3 ; then
	usage
fi

op=$1
in=$2
out=$3



for name in $names ; do
	indirre='$in/*_$name'
	indir=`eval ls -d $indirre 2>/dev/null`
	if test ! -z $indir && test -d $indir ; then
		outdir="$out/$name/$op"
		echo "'$indir'" '>' "'$outdir'"
		echo -n 'Copying?>' ; read input

		if test -z $input ; then
			mkdir -p $outdir
			ls "$indir" | \
				xargs -I replace-str cp -r -f $indir/replace-str $outdir
			echo -n 'Sending?>' ; read input
			pwd=`pwd` 
			if test -z $input ; then
				cd /var/www
				bash ./sspvo.sh cron
			fi
			cd "$pwd"
		fi

	fi
done

